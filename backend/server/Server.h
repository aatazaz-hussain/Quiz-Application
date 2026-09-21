#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

class Request {
public:
    std::string method;
    std::string path;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query;
    std::unordered_map<std::string, std::string> params;
    
    std::string getHeader(const std::string& name) const;
    std::string getQuery(const std::string& name) const;
    std::string getParam(const std::string& name) const;
};

class Response {
public:
    int status;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    
    Response();
    Response(int status, const std::string& body);
    
    void setHeader(const std::string& name, const std::string& value);
    void json(const std::string& jsonStr);
    void text(const std::string& text);
    void error(int code, const std::string& message);
};

class Server {
private:
    int port;
    bool running;
    std::string staticFolder;
    std::unordered_map<std::string, std::unordered_map<std::string, std::function<Response(const Request&)>>> routes;
    std::unordered_map<std::string, std::string> mimeTypes;
    
    void setupMimeTypes();
    std::string getMimeType(const std::string& filePath);
    std::string urlDecode(const std::string& encoded);
    Response serveStaticFile(const std::string& filePath);
    void handleClient(int clientSocket);
    void sendResponse(int clientSocket, const Response& res);
    std::string getStatusText(int status);
    
public:
    Server(int port);
    ~Server();
    
    void get(const std::string& path, std::function<Response(const Request&)> handler);
    void post(const std::string& path, std::function<Response(const Request&)> handler);
    void put(const std::string& path, std::function<Response(const Request&)> handler);
    void del(const std::string& path, std::function<Response(const Request&)> handler);
    
    void staticFiles(const std::string& folderPath);
    
    void start();
    void stop();
};

#endif // SERVER_H