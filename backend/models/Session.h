#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <ctime>

class Session {
private:
    int id;
    int userId;
    std::string token;
    std::string userType;
    std::string createdAt;
    std::string expiresAt;
    bool isValid;

public:
    Session();
    Session(int id, int userId, std::string token, std::string userType);
    
    int getId() const;
    int getUserId() const;
    std::string getToken() const;
    std::string getUserType() const;
    std::string getCreatedAt() const;
    std::string getExpiresAt() const;
    bool getIsValid() const;
    
    void setUserId(int userId);
    void setToken(std::string token);
    void setUserType(std::string userType);
    void invalidate();
    bool isExpired() const;
    
    bool save();
    bool update();
    static Session findByToken(std::string token);
    static Session findByUserId(int userId);
    static bool deleteByToken(std::string token);
    static bool deleteByUserId(int userId);
    
    std::string toJson() const;
    static Session fromJson(std::string jsonStr);
};

#endif