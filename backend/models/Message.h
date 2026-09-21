#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <ctime>

class Notification {
private:
    int id;
    int userId;
    std::string content;
    std::string type;
    bool isRead;
    std::string createdAt;

public:
    Notification();
    Notification(int id, int userId, std::string content, std::string type);
    
    int getId() const;
    int getUserId() const;
    std::string getContent() const;
    std::string getType() const;
    bool getIsRead() const;
    std::string getCreatedAt() const;
    
    void setContent(std::string content);
    void markAsRead();
    
    bool save();
    static Notification findById(int notificationId);
    static std::vector<Notification> findByUser(int userId, bool unreadOnly);
    static bool markAllAsRead(int userId);
    
    std::string toJson() const;
    static Notification fromJson(std::string jsonStr);
    void setUserId(int userId);
void setType(std::string type);
void setIsRead(bool isRead);
    static bool markAsRead(int notificationId);
static std::vector<Notification> getByUserId(int userId); 
};

class Message {
private:
    int id;
    int senderId;
    int receiverId;
    std::string content;
    std::string senderType;
    std::string receiverType;
    bool isRead;
    std::string createdAt;

public:
    Message();
    Message(int id, int senderId, int receiverId, std::string content);
    
    int getId() const;
    int getSenderId() const;
    int getReceiverId() const;
    std::string getContent() const;
    std::string getSenderType() const;
    std::string getReceiverType() const;
    bool getIsRead() const;
    std::string getCreatedAt() const;
    
    void setContent(std::string content);
    void markAsRead();
     void setId(int id);
    void setCreatedAt(const std::string& createdAt);
    bool save();
    static Message findById(int messageId);
    static std::vector<Message> getConversation(int user1Id, int user2Id);
    static std::vector<Message> getInbox(int userId);
    static std::vector<Message> getSent(int userId);
    static int getUnreadCount(int userId);
    
    std::string toJson() const;
    static Message fromJson(std::string jsonStr);
    void setSenderId(int senderId);
void setReceiverId(int receiverId);
void setSenderType(std::string senderType);
void setReceiverType(std::string receiverType);
void setIsRead(bool isRead);
};

#endif