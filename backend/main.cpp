#include "database/Database.h"
#include "server/Server.h"
#include "models/User.h"
#include "models/Quiz.h"
#include "models/Result.h"
#include "models/Message.h"
#include "models/Session.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <sstream>
#include <random>
#include <algorithm>
#include <map>

std::string generateToken()
{
    std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);
    std::string token;
    for (int i = 0; i < 32; i++)
    {
        token += chars[dist(generator)];
    }
    return token;
}

std::string urlDecode(const std::string &encoded)
{
    std::string decoded;
    for (size_t i = 0; i < encoded.length(); i++)
    {
        if (encoded[i] == '%' && i + 2 < encoded.length())
        {
            int hex;
            std::istringstream hexStream(encoded.substr(i + 1, 2));
            if (hexStream >> std::hex >> hex)
            {
                decoded += static_cast<char>(hex);
                i += 2;
            }
            else
            {
                decoded += encoded[i];
            }
        }
        else if (encoded[i] == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += encoded[i];
        }
    }
    return decoded;
}

std::string normalizeUserType(const std::string &userType)
{
    if (userType == "Administrator")
        return "admin";
    if (userType == "teacher" || userType == "student")
        return userType;
    return userType;
}

std::map<int, int> parseAnswers(const std::string &answersStr)
{
    std::map<int, int> answers;

    // First, URL decode the string
    std::string decodedStr = urlDecode(answersStr);

    if (decodedStr.empty() || decodedStr == "demo" || decodedStr == "{}")
    {
        return answers;
    }

    // Debug: print what we received
    std::cout << "Parsing answers: " << decodedStr << std::endl;

    // Remove outer braces
    if (decodedStr[0] == '{')
        decodedStr = decodedStr.substr(1);
    if (decodedStr.back() == '}')
        decodedStr.pop_back();

    // Now parse the key-value pairs
    size_t pos = 0;
    while (pos < decodedStr.size())
    {
        // Find the key (question ID)
        size_t quote1 = decodedStr.find('"', pos);
        if (quote1 == std::string::npos)
            break;

        size_t quote2 = decodedStr.find('"', quote1 + 1);
        if (quote2 == std::string::npos)
            break;

        std::string keyStr = decodedStr.substr(quote1 + 1, quote2 - quote1 - 1);

        // Find colon after the key
        size_t colon = decodedStr.find(':', quote2);
        if (colon == std::string::npos)
            break;

        // Find the value (option index)
        size_t valueStart = colon + 1;
        while (valueStart < decodedStr.size() && (decodedStr[valueStart] == ' ' || decodedStr[valueStart] == '\t'))
        {
            valueStart++;
        }

        size_t valueEnd = valueStart;
        while (valueEnd < decodedStr.size() &&
               decodedStr[valueEnd] != ',' &&
               decodedStr[valueEnd] != '}' &&
               decodedStr[valueEnd] != ' ')
        {
            valueEnd++;
        }

        if (valueEnd <= valueStart)
            break;

        std::string valueStr = decodedStr.substr(valueStart, valueEnd - valueStart);

        // Convert to integers
        try
        {
            int questionId = std::stoi(keyStr);
            int optionIndex = std::stoi(valueStr);
            answers[questionId] = optionIndex;

            // Debug: print each parsed answer
            std::cout << "  Question " << questionId << " -> Option " << optionIndex << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing answer: " << keyStr << ":" << valueStr << " - " << e.what() << std::endl;
        }

        // Move to next pair
        pos = valueEnd;
        if (decodedStr[pos] == ',')
            pos++;
    }

    std::cout << "Total parsed answers: " << answers.size() << std::endl;
    return answers;
}

int main()
{
    Database::initialize();

    Server server(8080);
    server.staticFiles("E:/BSSE-5B/OOAD/project/Quiz Application/Frontend");

    server.post("/api/login", [](const Request &req)
                {
        Response res;

        std::string email = urlDecode(req.getQuery("email"));
        std::string password = urlDecode(req.getQuery("password"));
        std::string userType = urlDecode(req.getQuery("userType"));
        
        std::string backendUserType = normalizeUserType(userType);
        
        User user = User::authenticate(email, password);
        
        if(user.getId() == 0) {
            res.error(401, "Invalid credentials");
            return res;
        }
        
        if(user.getUserType() != backendUserType) {
            res.error(401, "Invalid user type");
            return res;
        }

        Session oldSession = Session::findByUserId(user.getId());
        if(oldSession.getId() != 0) {
            Session::deleteByUserId(user.getId());
        }

        Session session;
        session.setUserId(user.getId());
        session.setUserType(user.getUserType());
        session.setToken(generateToken());
        session.save();

        std::string json = "{\"success\":true,\"token\":\"" + session.getToken() + 
                            "\",\"user\":" + user.toJson() + "}";
        res.json(json);
        return res; });

    server.post("/api/register", [](const Request &req)
                {
        Response res;

        std::string name = urlDecode(req.getQuery("name"));
        std::string email = urlDecode(req.getQuery("email"));
        std::string password = urlDecode(req.getQuery("password"));
        std::string userType = urlDecode(req.getQuery("userType"));
        std::string extra = urlDecode(req.getQuery("extra"));

        User check = User::authenticate(email, "dummy");
        if(check.getId() != 0) {
            res.error(400, "User already exists");
            return res;
        }

        if(userType == "student") {
            Student student(0, name, email, extra);
            student.setPassword(password);
            student.save();
        } else if(userType == "teacher") {
            Teacher teacher(0, name, email, extra);
            teacher.setPassword(password);
            teacher.save();
        }

        res.json("{\"success\":true,\"message\":\"Registration successful\"}");
        return res; });
    server.get("/api/messages/conversation", [](const Request &req)
               {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0) {
        res.error(401, "Unauthorized");
        return res;
    }

    std::string otherUserIdStr = req.getQuery("userId");
    if(otherUserIdStr.empty()) {
        res.error(400, "User ID is required");
        return res;
    }

    int otherUserId = std::stoi(otherUserIdStr);
    
    // Use your existing Message class method
    std::vector<Message> messages = Message::getConversation(session.getUserId(), otherUserId);
    
    std::string json = "[";
    for(size_t i = 0; i < messages.size(); i++) {
        if(i > 0) json += ",";
        
        User sender = User::findById(messages[i].getSenderId());
        std::string senderName = sender.getId() != 0 ? sender.getName() : "Unknown";
        
        std::string content = messages[i].getContent();
        size_t pos = 0;
        while((pos = content.find('"', pos)) != std::string::npos) {
            content.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        json += "{\"id\":" + std::to_string(messages[i].getId()) +
                ",\"senderId\":" + std::to_string(messages[i].getSenderId()) +
                ",\"senderName\":\"" + senderName + "\"" +
                ",\"receiverId\":" + std::to_string(messages[i].getReceiverId()) +
                ",\"content\":\"" + content + "\"" +
                ",\"createdAt\":\"" + messages[i].getCreatedAt() + "\"" +
                ",\"senderType\":\"" + messages[i].getSenderType() + "\"}";
    }
    json += "]";
    
    if(messages.empty()) {
        json = "[]";
    }
    
    res.json(json);
    return res; });
    server.post("/api/messages/send-broadcast", [](const Request &req)
                {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0 || session.getUserType() != "teacher") {
        res.error(401, "Unauthorized");
        return res;
    }

    std::string content = urlDecode(req.getQuery("content"));
    if(content.empty()) {
        res.error(400, "Content is required");
        return res;
    }

    // Get all students
    std::vector<User> allUsers = User::getAll();
    int sentCount = 0;
    
    for(const auto& user : allUsers) {
        if(user.getUserType() == "student") {
            Message message;
            message.setSenderId(session.getUserId());
            message.setReceiverId(user.getId());
            message.setContent(content);
            message.setSenderType("teacher");
            message.setReceiverType("student");
            
            if(message.save()) {
                sentCount++;
            }
        }
    }
    
    if(sentCount > 0) {
        res.json("{\"success\":true,\"message\":\"Message sent to " + std::to_string(sentCount) + " students\"}");
    } else {
        res.error(500, "Failed to send message to any student");
    }
    
    return res; });
    server.get("/api/quizzes", [](const Request &req)
               {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0) {
        res.error(401, "Unauthorized");
        return res;
    }

    std::vector<Quiz> quizzes;
    if(session.getUserType() == "teacher") {
        quizzes = Quiz::findByTeacher(session.getUserId());
    } else {
        quizzes = Quiz::getAvailableForStudent(session.getUserId());
    }

    std::string json = "[";
    for(size_t i = 0; i < quizzes.size(); i++) {
        if(i > 0) json += ",";
        json += quizzes[i].toJson();
        
        // Get question count
        std::vector<Question> questions = Question::findByQuizId(quizzes[i].getId());
        
        // Get submission count for this quiz
        sqlite3* db = Database::getConnection();
        std::string countSql = "SELECT COUNT(*) FROM submissions WHERE quiz_id = ?";
        sqlite3_stmt* countStmt;
        int submissionCount = 0;
        
        if(sqlite3_prepare_v2(db, countSql.c_str(), -1, &countStmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(countStmt, 1, quizzes[i].getId());
            if(sqlite3_step(countStmt) == SQLITE_ROW) {
                submissionCount = sqlite3_column_int(countStmt, 0);
            }
            sqlite3_finalize(countStmt);
        }
        
        // Remove the closing brace from toJson() and add our extra fields
        if(!json.empty() && json.back() == '}') {
            json.pop_back();
        }
        json += ",\"questionCount\":" + std::to_string(questions.size()) + 
                ",\"submissions\":" + std::to_string(submissionCount) + "}";
    }
    json += "]";
    res.json(json);
    return res; });

    server.get("/api/quiz-questions", [](const Request &req)
               {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "student") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string quizIdStr = req.getQuery("quizId");
        if(quizIdStr.empty()) {
            res.error(400, "Quiz ID is required");
            return res;
        }

        int quizId = std::stoi(quizIdStr);
        
        Quiz quiz = Quiz::findById(quizId);
        if(quiz.getId() == 0) {
            res.error(404, "Quiz not found");
            return res;
        }
        
        if(quiz.getStatus() != "published") {
            res.error(403, "Quiz is not published");
            return res;
        }

        std::vector<Question> questions = Question::findByQuizId(quizId);
        
        std::string json = "{\"quizId\":" + std::to_string(quizId) + ",\"questions\":[";
        for(size_t i = 0; i < questions.size(); i++) {
            if(i > 0) json += ",";
            
            std::vector<std::string> options = questions[i].getOptions();
            
            json += "{\"id\":" + std::to_string(questions[i].getId()) + 
                    ",\"text\":\"" + questions[i].getText() + 
                    "\",\"type\":\"" + questions[i].getType() + 
                    "\",\"points\":" + std::to_string(questions[i].getPoints()) + 
                    ",\"options\":[";
            
            for (size_t j = 0; j < options.size(); j++) {
                if(j > 0) json += ",";
                std::string escapedOption = options[j];
                size_t pos = 0;
                while((pos = escapedOption.find('"', pos)) != std::string::npos) {
                    escapedOption.replace(pos, 1, "\\\"");
                    pos += 2;
                }
                json += "\"" + escapedOption + "\"";
            }
            json += "]}";
        }
        json += "]}";
        res.json(json);
        return res; });

    server.post("/api/quizzes", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "teacher") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string title = urlDecode(req.getQuery("title"));
        std::string desc = urlDecode(req.getQuery("description"));
        std::string subject = urlDecode(req.getQuery("subject"));

        Quiz quiz(0, title, desc, session.getUserId(), subject);
        quiz.save();
        res.json("{\"success\":true,\"quizId\":" + std::to_string(quiz.getId()) + "}");
        return res; });

    server.post("/api/submit-quiz", [](const Request &req)
                {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0 || session.getUserType() != "student") {
        res.error(401, "Unauthorized");
        return res;
    }

    std::string quizIdStr = urlDecode(req.getQuery("quizId"));
    std::string answersStr = urlDecode(req.getQuery("answers"));
    
    if(quizIdStr.empty()) {
        res.error(400, "Quiz ID is required");
        return res;
    }

    int quizId = std::stoi(quizIdStr);
    
    std::cout << "=== SUBMITTING QUIZ ===" << std::endl;
    std::cout << "Quiz ID: " << quizId << std::endl;
    std::cout << "Student ID: " << session.getUserId() << std::endl;
    
    std::map<int, int> studentAnswers = parseAnswers(answersStr);
    
    std::vector<Question> questions = Question::findByQuizId(quizId);
    
    std::cout << "Found " << questions.size() << " questions for quiz " << quizId << std::endl;
    
    if(questions.empty()) {
        res.error(404, "No questions found for this quiz");
        return res;
    }
    
    int totalPoints = 0;
    int earnedPoints = 0;
    
    for(const auto& question : questions) {
        totalPoints += question.getPoints();
        
        std::cout << "Question " << question.getId() << ": ";
        std::cout << "Correct option index = " << question.getCorrectOption() << ", ";
        std::cout << "Student's answer = ";
        
        auto it = studentAnswers.find(question.getId());
        if(it != studentAnswers.end()) {
            std::cout << it->second;
            if(it->second == question.getCorrectOption()) {
                earnedPoints += question.getPoints();
                std::cout << " (CORRECT, +" << question.getPoints() << " points)";
            } else {
                std::cout << " (WRONG)";
            }
        } else {
            std::cout << "NOT ANSWERED";
        }
        std::cout << std::endl;
    }
    
    int percentage = (totalPoints > 0) ? (earnedPoints * 100) / totalPoints : 0;
    
    std::cout << "Total points: " << totalPoints << std::endl;
    std::cout << "Earned points: " << earnedPoints << std::endl;
    std::cout << "Percentage: " << percentage << "%" << std::endl;
    
    // Save submission
    sqlite3* db = Database::getConnection();
    
    // First, save the submission
    std::string submissionSql = "INSERT INTO submissions (student_id, quiz_id, answers, submitted_at) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* submissionStmt;
    
    if(sqlite3_prepare_v2(db, submissionSql.c_str(), -1, &submissionStmt, NULL) != SQLITE_OK) {
        res.error(500, "Failed to prepare submission statement");
        return res;
    }
    
    sqlite3_bind_int(submissionStmt, 1, session.getUserId());
    sqlite3_bind_int(submissionStmt, 2, quizId);
    sqlite3_bind_text(submissionStmt, 3, answersStr.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    std::string nowStr = std::to_string(now);
    sqlite3_bind_text(submissionStmt, 4, nowStr.c_str(), -1, SQLITE_TRANSIENT);
    
    if(sqlite3_step(submissionStmt) != SQLITE_DONE) {
        sqlite3_finalize(submissionStmt);
        res.error(500, "Failed to save submission");
        return res;
    }
    
    int submissionId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(submissionStmt);
    
    std::cout << "Saved submission with ID: " << submissionId << std::endl;
    
    // Now save the result
    std::string resultSql = "INSERT INTO results (submission_id, student_id, quiz_id, total_score, max_score, graded_at) VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* resultStmt;
    
    if(sqlite3_prepare_v2(db, resultSql.c_str(), -1, &resultStmt, NULL) != SQLITE_OK) {
        res.error(500, "Failed to prepare result statement");
        return res;
    }
    
    sqlite3_bind_int(resultStmt, 1, submissionId);
    sqlite3_bind_int(resultStmt, 2, session.getUserId());
    sqlite3_bind_int(resultStmt, 3, quizId);
    sqlite3_bind_int(resultStmt, 4, earnedPoints);
    sqlite3_bind_int(resultStmt, 5, totalPoints);
    sqlite3_bind_text(resultStmt, 6, nowStr.c_str(), -1, SQLITE_TRANSIENT);
    
    if(sqlite3_step(resultStmt) != SQLITE_DONE) {
        sqlite3_finalize(resultStmt);
        res.error(500, "Failed to save result");
        return res;
    }
    
    int resultId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(resultStmt);
    
    std::cout << "Saved result with ID: " << resultId << std::endl;
    
    Quiz quiz = Quiz::findById(quizId);
    std::string quizTitle = quiz.getTitle();
    
    std::string json = "{\"success\":true,\"score\":" + std::to_string(percentage) + 
                       ",\"earnedPoints\":" + std::to_string(earnedPoints) + 
                       ",\"totalPoints\":" + std::to_string(totalPoints) + 
                       ",\"quizTitle\":\"" + quizTitle + "\"}";
    res.json(json);
    return res; });

    server.get("/api/messages", [](const Request &req)
               {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0) {
            res.error(401, "Unauthorized");
            return res;
        }

        std::vector<Message> messages;
        
        if(session.getUserType() == "student") {
            messages = Message::getInbox(session.getUserId());
            
            std::vector<Message> filteredMessages;
            for(const auto& msg : messages) {
                if((msg.getSenderId() == session.getUserId() && msg.getReceiverType() == "teacher") ||
                   (msg.getReceiverId() == session.getUserId() && msg.getSenderType() == "teacher")) {
                    filteredMessages.push_back(msg);
                }
            }
            messages = filteredMessages;
        } else {
            messages = Message::getInbox(session.getUserId());
        }
        
        std::string json = "[";
        for(size_t i = 0; i < messages.size(); i++) {
            if(i > 0) json += ",";
            
            User sender = User::findById(messages[i].getSenderId());
            std::string senderName = sender.getId() != 0 ? sender.getName() : "Unknown";
            
            std::string content = messages[i].getContent();
            size_t pos = 0;
            while((pos = content.find('"', pos)) != std::string::npos) {
                content.replace(pos, 1, "\\\"");
                pos += 2;
            }
            
            json += "{\"id\":" + std::to_string(messages[i].getId()) +
                    ",\"senderId\":" + std::to_string(messages[i].getSenderId()) +
                    ",\"senderName\":\"" + senderName + "\"" +
                    ",\"receiverId\":" + std::to_string(messages[i].getReceiverId()) +
                    ",\"content\":\"" + content + "\"" +
                    ",\"createdAt\":\"" + messages[i].getCreatedAt() + "\"" +
                    ",\"senderType\":\"" + messages[i].getSenderType() + "\"}";
        }
        json += "]";
        res.json(json);
        return res; });
    server.post("/api/messages/send", [](const Request &req)
                {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0) {
        res.error(401, "Unauthorized");
        return res;
    }

    // Debug output
    std::cout << "=== MESSAGE SEND REQUEST ===" << std::endl;
    
    std::string content;
    std::string teacherIdStr;
    std::string receiverIdStr;
    
    // METHOD 1: Try to get from POST body (URL-encoded form data)
    // Parse the body manually since we don't know the format
    if(!req.body.empty()) {
        std::cout << "Body received: " << req.body << std::endl;
        
        // Check if it's JSON
        if(req.body.find('{') != std::string::npos) {
            std::cout << "Body appears to be JSON" << std::endl;
            
            // Simple JSON parsing for content
            size_t contentPos = req.body.find("\"content\":\"");
            if(contentPos != std::string::npos) {
                size_t start = contentPos + 10; // Length of "\"content\":"
                size_t quoteStart = req.body.find('"', start);
                if(quoteStart != std::string::npos) {
                    size_t quoteEnd = req.body.find('"', quoteStart + 1);
                    if(quoteEnd != std::string::npos) {
                        content = req.body.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    }
                }
            }
            
            // Simple JSON parsing for teacherId
            size_t teacherIdPos = req.body.find("\"teacherId\":");
            if(teacherIdPos != std::string::npos) {
                size_t start = teacherIdPos + 11; // Length of "\"teacherId\":"
                while(start < req.body.length() && 
                      (req.body[start] == ' ' || req.body[start] == ':' || req.body[start] == '"')) {
                    start++;
                }
                size_t end = start;
                while(end < req.body.length() && 
                      req.body[end] != ',' && req.body[end] != '}' && req.body[end] != '"') {
                    end++;
                }
                teacherIdStr = req.body.substr(start, end - start);
            }
        } 
        // Check if it's URL-encoded (contains '=')
        else if(req.body.find('=') != std::string::npos) {
            std::cout << "Body appears to be URL-encoded" << std::endl;
            
            // Parse key=value pairs
            std::string bodyCopy = req.body;
            size_t pos = 0;
            
            while(pos < bodyCopy.length()) {
                // Find key
                size_t eqPos = bodyCopy.find('=', pos);
                if(eqPos == std::string::npos) break;
                
                // Find end of value (next & or end of string)
                size_t ampPos = bodyCopy.find('&', eqPos);
                if(ampPos == std::string::npos) ampPos = bodyCopy.length();
                
                std::string key = bodyCopy.substr(pos, eqPos - pos);
                std::string value = bodyCopy.substr(eqPos + 1, ampPos - eqPos - 1);
                
                std::cout << "Parsed key: " << key << ", value: " << value << std::endl;
                
                if(key == "content") {
                    content = urlDecode(value);
                } else if(key == "teacherId") {
                    teacherIdStr = value;
                } else if(key == "receiverId") {
                    receiverIdStr = value;
                }
                
                pos = ampPos + 1;
            }
        }
        // Otherwise, treat the entire body as content
        else {
            content = urlDecode(req.body);
        }
    }
    
    // METHOD 2: If still empty, try to get from query parameters (GET-style)
    if(content.empty()) {
        content = urlDecode(req.getQuery("content"));
        teacherIdStr = req.getQuery("teacherId");
        receiverIdStr = req.getQuery("receiverId");
    }
    
    std::cout << "Final parsed values:" << std::endl;
    std::cout << "  content: " << content << std::endl;
    std::cout << "  teacherId: " << teacherIdStr << std::endl;
    std::cout << "  receiverId: " << receiverIdStr << std::endl;
    
    if(content.empty()) {
        res.error(400, "Message content is required");
        return res;
    }
    
    int receiverId = 0;
    std::string receiverType = "";
    
    if(session.getUserType() == "student") {
        // Student sending to teacher
        receiverType = "teacher";
        
        if(!teacherIdStr.empty()) {
            try {
                receiverId = std::stoi(teacherIdStr);
            } catch(...) {
                // If conversion fails, find first teacher
                receiverId = 0;
            }
        } else if(!receiverIdStr.empty()) {
            try {
                receiverId = std::stoi(receiverIdStr);
            } catch(...) {
                receiverId = 0;
            }
        }
        
        // If still no receiverId, find first teacher
        if(receiverId == 0) {
            std::vector<User> allUsers = User::getAll();
            for(const auto& user : allUsers) {
                if(user.getUserType() == "teacher") {
                    receiverId = user.getId();
                    break;
                }
            }
        }
    } else if(session.getUserType() == "teacher") {
        // Teacher sending to student
        receiverType = "student";
        
        if(!receiverIdStr.empty()) {
            try {
                receiverId = std::stoi(receiverIdStr);
            } catch(...) {
                receiverId = 0;
            }
        }
        
        // If no receiverId specified, find first student
        if(receiverId == 0) {
            std::vector<User> allUsers = User::getAll();
            for(const auto& user : allUsers) {
                if(user.getUserType() == "student") {
                    receiverId = user.getId();
                    break;
                }
            }
        }
    }
    
    if(receiverId == 0) {
        res.error(400, "No recipient available");
        return res;
    }
    
    std::cout << "Sending message:" << std::endl;
    std::cout << "  From: " << session.getUserId() << " (" << session.getUserType() << ")" << std::endl;
    std::cout << "  To: " << receiverId << " (" << receiverType << ")" << std::endl;
    std::cout << "  Content: " << content << std::endl;
    
    // Create and save message
    Message message;
    message.setSenderId(session.getUserId());
    message.setReceiverId(receiverId);
    message.setContent(content);
    message.setSenderType(session.getUserType());
    message.setReceiverType(receiverType);
    message.setIsRead(false);
    
    if(message.save()) {
        std::cout << "Message saved successfully!" << std::endl;
        res.json("{\"success\":true,\"message\":\"Message sent successfully\"}");
    } else {
        std::cout << "Failed to save message!" << std::endl;
        res.error(500, "Failed to save message");
    }
    
    return res; });
    server.get("/api/teachers", [](const Request &req)
               {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0) {
        res.error(401, "Unauthorized");
        return res;
    }

    std::vector<User> allUsers = User::getAll();
    std::vector<User> teachers;
    
    for(const auto& user : allUsers) {
        if(user.getUserType() == "teacher") {
            teachers.push_back(user);
        }
    }
    
    std::string json = "[";
    for(size_t i = 0; i < teachers.size(); i++) {
        if(i > 0) json += ",";
        
        std::string name = teachers[i].getName();
        std::string email = teachers[i].getEmail();
        
        // Escape quotes
        size_t pos = 0;
        while((pos = name.find('"', pos)) != std::string::npos) {
            name.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        pos = 0;
        while((pos = email.find('"', pos)) != std::string::npos) {
            email.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        json += "{\"id\":" + std::to_string(teachers[i].getId()) + 
                ",\"name\":\"" + name + 
                "\",\"email\":\"" + email + "\"}";
    }
    json += "]";
    res.json(json);
    return res; });

    server.get("/api/students", [](const Request &req)
               {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0 || session.getUserType() != "teacher") {
        res.error(401, "Unauthorized");
        return res;
    }

    std::vector<User> allUsers = User::getAll();
    std::vector<User> students;
    
    for(const auto& user : allUsers) {
        if(user.getUserType() == "student") {
            students.push_back(user);
        }
    }
    
    std::string json = "[";
    for(size_t i = 0; i < students.size(); i++) {
        if(i > 0) json += ",";
        json += "{\"id\":" + std::to_string(students[i].getId()) + 
                ",\"name\":\"" + students[i].getName() + 
                "\",\"email\":\"" + students[i].getEmail() + "\"}";
    }
    json += "]";
    res.json(json);
    return res; });

    server.get("/api/results", [](const Request &req)
               {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0) {
            res.error(401, "Unauthorized");
            return res;
        }

        std::vector<Result> results = Result::findByStudent(session.getUserId());
        std::string json = "[";
        for(size_t i = 0; i < results.size(); i++) {
            if(i > 0) json += ",";
            
            Quiz quiz = Quiz::findById(results[i].getQuizId());
            std::string quizTitle = quiz.getTitle();
            
            int percentage = (results[i].getMaxScore() > 0) ? 
                (results[i].getTotalScore() * 100) / results[i].getMaxScore() : 0;
            
            json += "{\"id\":" + std::to_string(results[i].getId()) +
                    ",\"quizId\":" + std::to_string(results[i].getQuizId()) +
                    ",\"quizTitle\":\"" + quizTitle + "\"" +
                    ",\"totalScore\":" + std::to_string(results[i].getTotalScore()) +
                    ",\"maxScore\":" + std::to_string(results[i].getMaxScore()) +
                    ",\"percentage\":" + std::to_string(percentage) +
                    ",\"gradedAt\":\"" + results[i].getGradedAt() + "\"}";
        }
        json += "]";
        res.json(json);
        return res; });

    server.post("/api/logout", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session::deleteByToken(token);
        res.json("{\"success\":true}");
        return res; });

    server.get("/api/users", [](const Request &req)
               {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "admin") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::vector<User> users = User::getAll();
        std::string json = "[";
        for(size_t i = 0; i < users.size(); i++) {
            if(i > 0) json += ",";
            json += users[i].toJson();
        }
        json += "]";
        res.json(json);
        return res; });

    server.post("/api/delete-user", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "admin") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string userIdStr = req.getQuery("userId");
        if(userIdStr.empty()) {
            res.error(400, "User ID is required");
            return res;
        }

        int userId = std::stoi(userIdStr);
        
        Admin admin;
        bool success = admin.deleteUser(userId);
        
        if(success) res.json("{\"success\":true}");
        else res.error(500, "Failed to delete user");
        
        return res; });

    server.post("/api/quizzes/publish", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "teacher") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string quizIdStr = req.getQuery("quizId");
        if(quizIdStr.empty()) {
            res.error(400, "Quiz ID is required");
            return res;
        }

        int quizId = std::stoi(quizIdStr);
        Quiz quiz = Quiz::findById(quizId);
        
        if(quiz.getId() == 0) {
            res.error(404, "Quiz not found");
            return res;
        }

        if(quiz.getTeacherId() != session.getUserId()) {
            res.error(403, "You can only modify your own quizzes");
            return res;
        }

        if(quiz.getStatus() == "published") quiz.unpublish();
        else quiz.publish();
        
        if(quiz.update()) {
            std::string message = (quiz.getStatus() == "published") ? 
                                  "Quiz published successfully" : 
                                  "Quiz unpublished successfully";
            res.json("{\"success\":true,\"message\":\"" + message + "\"}");
        } else res.error(500, "Failed to update quiz status");
        
        return res; });

    server.post("/api/quizzes/update", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "teacher") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string quizIdStr = req.getQuery("quizId");
        if(quizIdStr.empty()) {
            res.error(400, "Quiz ID is required");
            return res;
        }

        int quizId = std::stoi(quizIdStr);
        Quiz quiz = Quiz::findById(quizId);
        
        if(quiz.getId() == 0) {
            res.error(404, "Quiz not found");
            return res;
        }

        if(quiz.getTeacherId() != session.getUserId()) {
            res.error(403, "You can only edit your own quizzes");
            return res;
        }

        std::string title = urlDecode(req.getQuery("title"));
        std::string description = urlDecode(req.getQuery("description"));
        std::string subject = urlDecode(req.getQuery("subject"));

        if(!title.empty()) quiz.setTitle(title);
        if(!description.empty()) quiz.setDescription(description);
        if(!subject.empty()) quiz.setSubject(subject);
        
        if(quiz.update()) res.json("{\"success\":true,\"message\":\"Quiz updated successfully\"}");
        else res.error(500, "Failed to update quiz");
        
        return res; });

    server.get("/api/questions", [](const Request &req)
               {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0) {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string quizIdStr = req.getQuery("quizId");
        if(quizIdStr.empty()) {
            res.error(400, "Quiz ID is required");
            return res;
        }

        int quizId = std::stoi(quizIdStr);
        
        Quiz quiz = Quiz::findById(quizId);
        if(quiz.getId() == 0) {
            res.error(404, "Quiz not found");
            return res;
        }
        
        if(session.getUserType() == "teacher" && quiz.getTeacherId() != session.getUserId()) {
            res.error(403, "Access denied");
            return res;
        }

        std::vector<Question> questions = Question::findByQuizId(quizId);
        
        std::string json = "[";
        for(size_t i = 0; i < questions.size(); i++) {
            if(i > 0) json += ",";
            std::vector<std::string> options = questions[i].getOptions();
            int correct = questions[i].getCorrectOption();
            
            json += "{\"id\":" + std::to_string(questions[i].getId()) + 
                    ",\"text\":\"" + questions[i].getText() + 
                    "\",\"type\":\"" + questions[i].getType() + 
                    "\",\"points\":" + std::to_string(questions[i].getPoints()) + 
                    ",\"quizId\":" + std::to_string(questions[i].getQuizId()) + 
                    ",\"options\":[";
            
            for (size_t j = 0; j < options.size(); j++) {
                if(j > 0) json += ",";
                json += "{";
                json += "\"id\":" + std::to_string(j) + ","; 
                json += "\"text\":\"" + options[j] + "\",";
                json += std::string("\"isCorrect\":") + (j == (size_t)correct ? "true" : "false");
                json += "}";
            }
            json += "]}";
        }
        json += "]";
        res.json(json);
        return res; });

    server.post("/api/questions", [](const Request &req)
                {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0 || session.getUserType() != "teacher") {
        res.error(401, "Unauthorized");
        return res;
    }

    // Parse JSON body instead of query parameters
std::string body = req.body;
    
    // Debug: Print the received body
    std::cout << "Received request body: " << body << std::endl;
    
    // Simple JSON parsing (you might want to use a proper JSON library)
    int quizId = 0;
    std::string text, type;
    int points = 1;
    std::vector<std::string> options;
    int correctOptionIndex = 0;
    
    // Parse JSON manually
    size_t pos = 0;
    
    // Extract quizId
    pos = body.find("\"quizId\":");
    if(pos != std::string::npos) {
        size_t start = body.find(':', pos) + 1;
        size_t end = body.find_first_of(",}", start);
        std::string quizIdStr = body.substr(start, end - start);
        quizId = std::stoi(quizIdStr);
    }
    
    // Extract text
    pos = body.find("\"text\":\"");
    if(pos != std::string::npos) {
        size_t start = pos + 8; // Length of "\"text\":\""
        size_t end = body.find('\"', start);
        text = body.substr(start, end - start);
    }
    
    // Extract type
    pos = body.find("\"type\":\"");
    if(pos != std::string::npos) {
        size_t start = pos + 8; // Length of "\"type\":\""
        size_t end = body.find('\"', start);
        type = body.substr(start, end - start);
    }
    
    // Extract points
    pos = body.find("\"points\":");
    if(pos != std::string::npos) {
        size_t start = body.find(':', pos) + 1;
        size_t end = body.find_first_of(",}", start);
        std::string pointsStr = body.substr(start, end - start);
        points = std::stoi(pointsStr);
    }
    
    // Extract options array
    pos = body.find("\"options\":[");
    if(pos != std::string::npos) {
        size_t start = pos + 11; // Length of "\"options\":["
        size_t end = body.find(']', start);
        std::string optionsArray = body.substr(start, end - start);
        
        // Parse array elements
        std::istringstream ss(optionsArray);
        std::string item;
        bool inQuotes = false;
        std::string current;
        
        for(char c : optionsArray) {
            if(c == '"' && !inQuotes) {
                inQuotes = true;
            } else if(c == '"' && inQuotes) {
                inQuotes = false;
                if(!current.empty()) {
                    options.push_back(current);
                    current.clear();
                }
            } else if(c == ',' && !inQuotes) {
                if(!current.empty()) {
                    options.push_back(current);
                    current.clear();
                }
            } else if(inQuotes) {
                current += c;
            }
        }
        
        if(!current.empty()) {
            options.push_back(current);
        }
    }
    
    // Extract correctOptionIndex
    pos = body.find("\"correctOptionIndex\":");
    if(pos != std::string::npos) {
        size_t start = body.find(':', pos) + 1;
        size_t end = body.find_first_of(",}", start);
        std::string indexStr = body.substr(start, end - start);
        correctOptionIndex = std::stoi(indexStr);
    }

    if(quizId == 0 || text.empty() || type.empty()) {
        std::cout << "Missing required fields: quizId=" << quizId 
                  << ", text=" << text << ", type=" << type << std::endl;
        res.error(400, "Missing required fields");
        return res;
    }

    Quiz quiz = Quiz::findById(quizId);

    if(quiz.getId() == 0 || quiz.getTeacherId() != session.getUserId()) {
        res.error(403, "Access denied");
        return res;
    }

    // If we still have no options, create default ones
    if(options.empty()) {
        if(type == "true_false") {
            options.push_back("True");
            options.push_back("False");
            if(correctOptionIndex >= 2) correctOptionIndex = 0;
        } else {
            options.push_back("Option 1");
            options.push_back("Option 2");
            options.push_back("Option 3");
            if(type == "multiple_choice" || type == "single_choice") {
                options.push_back("Option 4");
            }
        }
    }
    
    // Validate that correctIndex is within bounds
    if(correctOptionIndex < 0 || correctOptionIndex >= (int)options.size()) {
        correctOptionIndex = 0;
    }

    std::cout << "Saving question: " << text << std::endl;
    std::cout << "Options count: " << options.size() << std::endl;
    for(size_t i = 0; i < options.size(); i++) {
        std::cout << "  Option " << i << ": " << options[i] 
                  << (i == (size_t)correctOptionIndex ? " (correct)" : "") << std::endl;
    }

    Question question(0, quizId, text, type, points, options, correctOptionIndex);

    if(!question.save()) {
        res.error(500, "Failed to save question");
        return res;
    }

    res.json("{\"success\":true,\"questionId\":" + std::to_string(question.getId()) + "}");
    return res; });
    server.get("/api/teacher/results", [](const Request &req)
               {
    Response res;

    std::string token = req.getHeader("Authorization");
    Session session = Session::findByToken(token);
    if(session.getId() == 0 || session.getUserType() != "teacher") {
        res.error(401, "Unauthorized");
        return res;
    }

    std::vector<Quiz> quizzes = Quiz::findByTeacher(session.getUserId());
    std::vector<std::string> resultsJson;
    
    for(const auto& quiz : quizzes) {
        // Use the database directly to get results for this quiz
        sqlite3* db = Database::getConnection();
        std::string sql = "SELECT * FROM results WHERE quiz_id = ?";
        sqlite3_stmt* stmt;
        
        if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            continue;
        }
        
        sqlite3_bind_int(stmt, 1, quiz.getId());
        
        while(sqlite3_step(stmt) == SQLITE_ROW) {
            int resultId = sqlite3_column_int(stmt, 0);
            int submissionId = sqlite3_column_int(stmt, 1);
            int studentId = sqlite3_column_int(stmt, 2);
            int quizId = sqlite3_column_int(stmt, 3);
            int totalScore = sqlite3_column_int(stmt, 4);
            int maxScore = sqlite3_column_int(stmt, 5);
            
            // Get student name
            User student = User::findById(studentId);
            std::string studentName = student.getId() != 0 ? student.getName() : ("Student " + std::to_string(studentId));
            
            // Calculate percentage
            int percentage = (maxScore > 0) ? (totalScore * 100) / maxScore : 0;
            
            // Get submission date
            std::string gradedAt = "";
            
            // Try to get from submissions table
            std::string submissionSql = "SELECT submitted_at FROM submissions WHERE id = ?";
            sqlite3_stmt* submissionStmt;
            if(sqlite3_prepare_v2(db, submissionSql.c_str(), -1, &submissionStmt, NULL) == SQLITE_OK) {
                sqlite3_bind_int(submissionStmt, 1, submissionId);
                if(sqlite3_step(submissionStmt) == SQLITE_ROW) {
                    const char* dateStr = reinterpret_cast<const char*>(sqlite3_column_text(submissionStmt, 0));
                    if(dateStr) {
                        gradedAt = dateStr;
                    }
                }
                sqlite3_finalize(submissionStmt);
            }
            
            // If no date found, use current date
            if(gradedAt.empty()) {
                time_t now = time(0);
                struct tm* now_tm = localtime(&now);
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now_tm);
                gradedAt = buffer;
            }
            
            // Escape strings
            std::string escapedStudentName = studentName;
            size_t pos = 0;
            while((pos = escapedStudentName.find('"', pos)) != std::string::npos) {
                escapedStudentName.replace(pos, 1, "\\\"");
                pos += 2;
            }
            
            std::string escapedQuizTitle = quiz.getTitle();
            pos = 0;
            while((pos = escapedQuizTitle.find('"', pos)) != std::string::npos) {
                escapedQuizTitle.replace(pos, 1, "\\\"");
                pos += 2;
            }
            
            std::string resultJson = 
                "{\"id\":" + std::to_string(resultId) +
                ",\"studentId\":" + std::to_string(studentId) +
                ",\"studentName\":\"" + escapedStudentName + "\"" +
                ",\"quizId\":" + std::to_string(quizId) +
                ",\"quizTitle\":\"" + escapedQuizTitle + "\"" +
                ",\"totalScore\":" + std::to_string(totalScore) +
                ",\"maxScore\":" + std::to_string(maxScore) +
                ",\"percentage\":" + std::to_string(percentage) +
                ",\"submissionDate\":\"" + gradedAt + "\"}";
            resultsJson.push_back(resultJson);
        }
        
        sqlite3_finalize(stmt);
    }
    
    std::string json = "[";
    for(size_t i = 0; i < resultsJson.size(); i++) {
        if(i > 0) json += ",";
        json += resultsJson[i];
    }
    json += "]";
    
    res.json(json);
    return res; });

    server.post("/api/notifications/send", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0 || session.getUserType() != "teacher") {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string content = urlDecode(req.getQuery("content"));
        if(content.empty()) {
            res.error(400, "Content is required");
            return res;
        }

        std::vector<User> allUsers = User::getAll();
        int sentCount = 0;
        
        for(const auto& user : allUsers) {
            if(user.getUserType() == "student") {
                Notification notification;
                notification.setUserId(user.getId());
                notification.setContent(content);
                notification.setType("teacher_notification");
                notification.setIsRead(false);

                if(notification.save()) {
                    sentCount++;
                }
            }
        }
        
        if(sentCount > 0) {
            res.json("{\"success\":true,\"message\":\"Notification sent to " + std::to_string(sentCount) + " students\"}");
        } else {
            res.error(500, "Failed to send notification");
        }
        
        return res; });

    server.get("/api/notifications", [](const Request &req)
               {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0) {
            res.error(401, "Unauthorized");
            return res;
        }

        std::vector<Notification> notifications = Notification::findByUser(session.getUserId(), false);
        
        std::string json = "[";
        for(size_t i = 0; i < notifications.size(); i++) {
            if(i > 0) json += ",";
            json += notifications[i].toJson();
        }
        json += "]";
        
        res.json(json);
        return res; });

    server.post("/api/notifications/mark-read", [](const Request &req)
                {
        Response res;

        std::string token = req.getHeader("Authorization");
        Session session = Session::findByToken(token);
        if(session.getId() == 0) {
            res.error(401, "Unauthorized");
            return res;
        }

        std::string notificationIdStr = req.getQuery("notificationId");
        if(notificationIdStr.empty()) {
            res.error(400, "Notification ID is required");
            return res;
        }

        int notificationId = std::stoi(notificationIdStr);
        
        if(Notification::markAsRead(notificationId)) {
            res.json("{\"success\":true}");
        } else {
            res.error(500, "Failed to mark notification as read");
        }
        
        return res; });

    std::cout << "Starting QuizMaster Server on http://localhost:8080" << std::endl;
    server.start();
    return 0;
}