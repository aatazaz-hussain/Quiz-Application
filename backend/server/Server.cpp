#include "Server.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <cstring>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define getcwd _getcwd
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <dirent.h>
#endif

// Enable debugging
#define DEBUG 1

std::string Request::getHeader(const std::string& name) const {
    auto it = headers.find(name);
    return it != headers.end() ? it->second : "";
}

std::string Request::getQuery(const std::string& name) const {
    auto it = query.find(name);
    return it != query.end() ? it->second : "";
}

std::string Request::getParam(const std::string& name) const {
    auto it = params.find(name);
    return it != params.end() ? it->second : "";
}

Response::Response() : status(200) {
    headers["Content-Type"] = "text/plain";
}

Response::Response(int status, const std::string& body) : status(status), body(body) {
    headers["Content-Type"] = "text/plain";
}

void Response::setHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

void Response::json(const std::string& jsonStr) {
    status = 200;
    headers["Content-Type"] = "application/json";
    body = jsonStr;
}

void Response::text(const std::string& text) {
    status = 200;
    headers["Content-Type"] = "text/plain";
    body = text;
}

void Response::error(int code, const std::string& message) {
    status = code;
    headers["Content-Type"] = "application/json";
    body = "{\"error\":\"" + message + "\"}";
}

Server::Server(int port) : port(port), running(false), staticFolder("") {
    setupMimeTypes();
}

Server::~Server() {
    stop();
}

void Server::setupMimeTypes() {
    mimeTypes[".html"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".svg"] = "image/svg+xml";
    mimeTypes[".ttf"] = "font/ttf";
    mimeTypes[".woff"] = "font/woff";
    mimeTypes[".woff2"] = "font/woff2";
}

std::string Server::getMimeType(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if(dotPos == std::string::npos) return "application/octet-stream";
    
    std::string ext = filePath.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    auto it = mimeTypes.find(ext);
    return it != mimeTypes.end() ? it->second : "application/octet-stream";
}

std::string Server::urlDecode(const std::string& encoded) {
    std::string decoded;
    for(size_t i = 0; i < encoded.length(); i++) {
        if(encoded[i] == '%' && i + 2 < encoded.length()) {
            int hex;
            std::istringstream hexStream(encoded.substr(i + 1, 2));
            if(hexStream >> std::hex >> hex) {
                decoded += static_cast<char>(hex);
                i += 2;
            } else {
                decoded += encoded[i];
            }
        } else if(encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}

Response Server::serveStaticFile(const std::string& filePath) {
    if(staticFolder.empty()) {
        return Response(404, "Static files not configured");
    }
    
    // Decode URL-encoded characters
    std::string decodedPath = urlDecode(filePath);
    
    std::string fullPath = staticFolder;
    if(!decodedPath.empty() && decodedPath != "/") {
        // Remove leading slash if present
        std::string cleanPath = decodedPath;
        if(cleanPath[0] == '/') {
            cleanPath = cleanPath.substr(1);
        }
        fullPath += "/" + cleanPath;
    } else {
        fullPath += "/index.html";
    }
    
    #ifdef DEBUG
    std::cout << "DEBUG serveStaticFile:" << std::endl;
    std::cout << "  Request path: " << filePath << std::endl;
    std::cout << "  Decoded path: " << decodedPath << std::endl;
    std::cout << "  Static folder: " << staticFolder << std::endl;
    std::cout << "  Full path: " << fullPath << std::endl;
    #endif
    
    // Check if file exists
    std::ifstream file(fullPath, std::ios::binary);
    if(!file) {
        #ifdef DEBUG
        std::cout << "  File not found: " << fullPath << std::endl;
        #endif
        
        // Try with .html extension if no extension
        if(filePath.find('.') == std::string::npos) {
            std::string htmlPath = fullPath + ".html";
            std::ifstream htmlFile(htmlPath, std::ios::binary);
            if(htmlFile) {
                #ifdef DEBUG
                std::cout << "  Found as: " << htmlPath << std::endl;
                #endif
                std::stringstream buffer;
                buffer << htmlFile.rdbuf();
                
                Response response;
                response.status = 200;
                response.setHeader("Content-Type", getMimeType(htmlPath));
                response.body = buffer.str();
                return response;
            }
        }
        
        return Response(404, "File not found: " + fullPath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    Response response;
    response.status = 200;
    response.setHeader("Content-Type", getMimeType(fullPath));
    response.body = buffer.str();
    
    #ifdef DEBUG
    std::cout << "  File served successfully" << std::endl;
    #endif
    
    return response;
}

void Server::get(const std::string& path, std::function<Response(const Request&)> handler) {
    routes["GET"][path] = handler;
}

void Server::post(const std::string& path, std::function<Response(const Request&)> handler) {
    routes["POST"][path] = handler;
}

void Server::put(const std::string& path, std::function<Response(const Request&)> handler) {
    routes["PUT"][path] = handler;
}

void Server::del(const std::string& path, std::function<Response(const Request&)> handler) {
    routes["DELETE"][path] = handler;
}

void Server::staticFiles(const std::string& folderPath) {
    staticFolder = folderPath;
    
    // Replace backslashes with forward slashes for consistency
    std::replace(staticFolder.begin(), staticFolder.end(), '\\', '/');
    
    // Remove trailing slash if present
    if(!staticFolder.empty() && staticFolder.back() == '/') {
        staticFolder.pop_back();
    }
    
    #ifdef DEBUG
    std::cout << "Static files configured at: " << staticFolder << std::endl;
    
    // Check if folder exists
    std::ifstream test(staticFolder + "/index.html");
    if(test) {
        std::cout << "✓ Found index.html in static folder" << std::endl;
    } else {
        std::cout << "✗ index.html not found in static folder" << std::endl;
    }
    #endif
}

void Server::start() {
    // Initialize Winsock on Windows
    #ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }
    #endif

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #ifdef _WIN32
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return;
    }
    #else
    if (serverSocket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }
    #endif

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    #ifdef _WIN32
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed for port " << port << std::endl;
        close(serverSocket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return;
    }

    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(serverSocket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return;
    }

    running = true;
    std::cout << "\n==========================================" << std::endl;
    std::cout << "   QuizMaster Server Started Successfully!" << std::endl;
    std::cout << "   Port: " << port << std::endl;
    std::cout << "   URL: http://localhost:" << port << std::endl;
    std::cout << "   Static Folder: " << staticFolder << std::endl;
    std::cout << "==========================================\n" << std::endl;

    while (running) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket >= 0) {
            std::thread([this, clientSocket]() {
                handleClient(clientSocket);
                close(clientSocket);
            }).detach();
        }
    }

    close(serverSocket);
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void Server::stop() {
    running = false;
}

void Server::handleClient(int clientSocket) {
    char buffer[8192]; // Increased buffer size
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) return;

    buffer[bytesRead] = '\0';
    std::string requestStr(buffer);

    #ifdef DEBUG
    std::cout << "\n=== New Request ===" << std::endl;
    std::cout << requestStr << std::endl;
    #endif

    Request req;
    Response res;

    // Parse request line
    size_t methodEnd = requestStr.find(' ');
    size_t pathEnd = requestStr.find(' ', methodEnd + 1);

    if (methodEnd != std::string::npos && pathEnd != std::string::npos) {
        req.method = requestStr.substr(0, methodEnd);
        std::string fullPath = requestStr.substr(methodEnd + 1, pathEnd - methodEnd - 1);

        size_t queryPos = fullPath.find('?');
        if (queryPos != std::string::npos) {
            req.path = fullPath.substr(0, queryPos);
            std::string queryStr = fullPath.substr(queryPos + 1);

            std::stringstream qss(queryStr);
            std::string pair;
            while (std::getline(qss, pair, '&')) {
                size_t eqPos = pair.find('=');
                if (eqPos != std::string::npos) {
                    std::string key = pair.substr(0, eqPos);
                    std::string value = pair.substr(eqPos + 1);
                    req.query[key] = value;
                }
            }
        } else {
            req.path = fullPath;
        }

        // Parse headers
        size_t headersEnd = requestStr.find("\r\n\r\n");
        if (headersEnd != std::string::npos) {
            std::string headersStr = requestStr.substr(0, headersEnd);
            std::stringstream hss(headersStr);
            std::string line;
            
            // Skip request line
            std::getline(hss, line);
            
            while (std::getline(hss, line)) {
                if (line.empty() || line == "\r") continue;
                
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = line.substr(0, colonPos);
                    std::string value = line.substr(colonPos + 1);
                    
                    // Trim whitespace
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    
                    if (!value.empty() && value.back() == '\r') {
                        value.pop_back();
                    }
                    
                    req.headers[key] = value;
                }
            }
            
            // Parse body
            size_t bodyPos = headersEnd + 4;
            if (bodyPos < requestStr.length()) {
                req.body = requestStr.substr(bodyPos);
            }
        }
    } else {
        res = Response(400, "Bad Request");
        sendResponse(clientSocket, res);
        return;
    }

    #ifdef DEBUG
    std::cout << "Parsed Request:" << std::endl;
    std::cout << "  Method: " << req.method << std::endl;
    std::cout << "  Path: " << req.path << std::endl;
    #endif

    // Handle CORS preflight requests
    if (req.method == "OPTIONS") {
        res.status = 200;
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
        res.setHeader("Access-Control-Max-Age", "86400");
        sendResponse(clientSocket, res);
        return;
    }

    bool found = false;
    
    // Check exact route match first
    auto methodRoutes = routes.find(req.method);
    if (methodRoutes != routes.end()) {
        auto exactRoute = methodRoutes->second.find(req.path);
        if (exactRoute != methodRoutes->second.end()) {
            res = exactRoute->second(req);
            found = true;
        } else {
            // Check for wildcard/static file routes
            // First check if we should serve static files
            if (!staticFolder.empty()) {
                // Check if this looks like a static file request
                // (contains a file extension or is a common static path)
                bool isStaticRequest = false;
                
                // Check for file extension
                size_t dotPos = req.path.find_last_of('.');
                if (dotPos != std::string::npos) {
                    std::string ext = req.path.substr(dotPos);
                    if (mimeTypes.find(ext) != mimeTypes.end()) {
                        isStaticRequest = true;
                    }
                }
                
                // Check if it's a common static file path
                if (req.path == "/" || req.path.find("/css/") == 0 || 
                    req.path.find("/js/") == 0 || req.path.find("/images/") == 0 ||
                    req.path.find("/fonts/") == 0 || req.path.find(".html") != std::string::npos) {
                    isStaticRequest = true;
                }
                
                if (isStaticRequest) {
                    res = serveStaticFile(req.path);
                    found = true;
                }
            }
        }
    }

    if (!found) {
        // Check for API routes with similar patterns
        if (req.path.find("/api/") == 0) {
            res = Response(404, "API endpoint not found: " + req.path);
        } else if (!staticFolder.empty()) {
            // Try to serve as static file anyway
            res = serveStaticFile(req.path);
            if (res.status == 200) {
                found = true;
            }
        }
        
        if (!found) {
            res = Response(404, "Not Found");
        }
    }

    // Add CORS headers to all responses (except OPTIONS which we already handled)
    if (req.method != "OPTIONS") {
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    }

    sendResponse(clientSocket, res);
}

void Server::sendResponse(int clientSocket, const Response& res) {
    std::string responseStr = "HTTP/1.1 " + std::to_string(res.status) + " " + getStatusText(res.status) + "\r\n";

    for (auto& header : res.headers) {
        responseStr += header.first + ": " + header.second + "\r\n";
    }

    responseStr += "Content-Length: " + std::to_string(res.body.length()) + "\r\n";
    responseStr += "Connection: close\r\n\r\n";
    responseStr += res.body;

    #ifdef DEBUG
    std::cout << "Response: " << res.status << " " << getStatusText(res.status) << std::endl;
    std::cout << "Content-Length: " << res.body.length() << std::endl;
    if (res.body.length() < 200) {
        std::cout << "Body: " << res.body << std::endl;
    }
    std::cout << "=== End Request ===\n" << std::endl;
    #endif

    send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
}

std::string Server::getStatusText(int status) {
    switch(status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}