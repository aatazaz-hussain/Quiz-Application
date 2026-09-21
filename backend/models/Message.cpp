#include "Message.h"
#include <sqlite3.h>
#include <sstream>
#include "database/Database.h"


Notification::Notification() : id(0), userId(0), isRead(false), type("system") {}

Notification::Notification(int id, int userId, std::string content, std::string type)
    : id(id), userId(userId), content(content), type(type), isRead(false) {}

int Notification::getId() const { return id; }
int Notification::getUserId() const { return userId; }
std::string Notification::getContent() const { return content; }
std::string Notification::getType() const { return type; }
bool Notification::getIsRead() const { return isRead; }
std::string Notification::getCreatedAt() const { return createdAt; }

void Notification::setUserId(int userId) { this->userId = userId; }
void Notification::setContent(std::string content) { this->content = content; }
void Notification::setType(std::string type) { this->type = type; }
void Notification::setIsRead(bool isRead) { this->isRead = isRead; }
void Notification::markAsRead() { isRead = true; }

bool Notification::save() {
    sqlite3* db = Database::getConnection();
    if (!db) return false;
    
    std::string sql = "INSERT INTO notifications (user_id, content, type, is_read, created_at) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, isRead ? 1 : 0);
    
    time_t now = time(0);
    createdAt = std::to_string(now);
    sqlite3_bind_text(stmt, 5, createdAt.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return result;
}

Notification Notification::findById(int notificationId) {
   sqlite3* db = Database::getConnection();
   if (!db) return Notification();
   
 std::string sql = "SELECT id, user_id, content, type, is_read, created_at FROM notifications WHERE id = ?";
    sqlite3_stmt* stmt;
    Notification notification;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return notification;
    
    sqlite3_bind_int(stmt, 1, notificationId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        notification.id = sqlite3_column_int(stmt, 0);
        notification.userId = sqlite3_column_int(stmt, 1);
        notification.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        notification.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        notification.isRead = sqlite3_column_int(stmt, 4) == 1;
        notification.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }
    
    sqlite3_finalize(stmt);
    return notification;
}

std::vector<Notification> Notification::findByUser(int userId, bool unreadOnly) {
  sqlite3* db = Database::getConnection();
  if (!db) return std::vector<Notification>();
  
  std::vector<Notification> notifications;
    std::string sql = "SELECT id, user_id, content, type, is_read, created_at FROM notifications WHERE user_id = ?";
    
    if(unreadOnly) {
        sql += " AND is_read = 0";
    }
    
    sql += " ORDER BY created_at DESC";
    
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return notifications;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Notification notification;
        notification.id = sqlite3_column_int(stmt, 0);
        notification.userId = sqlite3_column_int(stmt, 1);
        notification.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        notification.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        notification.isRead = sqlite3_column_int(stmt, 4) == 1;
        notification.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        notifications.push_back(notification);
    }
    
    sqlite3_finalize(stmt);
    return notifications;
}

bool Notification::markAllAsRead(int userId) {
   sqlite3* db = Database::getConnection();
   if (!db) return false;
   
 std::string sql = "UPDATE notifications SET is_read = 1 WHERE user_id = ? AND is_read = 0";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool Notification::markAsRead(int notificationId) {
    sqlite3* db = Database::getConnection();
    if (!db) return false;
    
    std::string sql = "UPDATE notifications SET is_read = 1 WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, notificationId);
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

std::vector<Notification> Notification::getByUserId(int userId) {
    return findByUser(userId, false);
}

std::string Notification::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"userId\":" << userId << ",";
    json << "\"content\":\"" << content << "\",";
    json << "\"type\":\"" << type << "\",";
    json << "\"isRead\":" << (isRead ? "true" : "false") << ",";
    json << "\"createdAt\":\"" << createdAt << "\"";
    json << "}";
    return json.str();
}

Notification Notification::fromJson(std::string jsonStr) {
    Notification notification;
    return notification;
}

Message::Message() : id(0), senderId(0), receiverId(0), isRead(false) {}

Message::Message(int id, int senderId, int receiverId, std::string content)
    : id(id), senderId(senderId), receiverId(receiverId), content(content), isRead(false) {}

int Message::getId() const { return id; }
int Message::getSenderId() const { return senderId; }
int Message::getReceiverId() const { return receiverId; }
std::string Message::getContent() const { return content; }
std::string Message::getSenderType() const { return senderType; }
std::string Message::getReceiverType() const { return receiverType; }
bool Message::getIsRead() const { return isRead; }
std::string Message::getCreatedAt() const { return createdAt; }

void Message::setSenderId(int senderId) { this->senderId = senderId; }
void Message::setReceiverId(int receiverId) { this->receiverId = receiverId; }
void Message::setContent(std::string content) { this->content = content; }
void Message::setSenderType(std::string senderType) { this->senderType = senderType; }
void Message::setReceiverType(std::string receiverType) { this->receiverType = receiverType; }
void Message::setIsRead(bool isRead) { this->isRead = isRead; }
void Message::markAsRead() { isRead = true; }

bool Message::save() {
   sqlite3* db = Database::getConnection();
   if (!db) return false;
   
 std::string sql = "INSERT INTO messages (sender_id, receiver_id, content, sender_type, receiver_type, is_read, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, senderId);
    sqlite3_bind_int(stmt, 2, receiverId);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, senderType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, receiverType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, isRead ? 1 : 0);
    
    time_t now = time(0);
    createdAt = std::to_string(now);
    sqlite3_bind_text(stmt, 7, createdAt.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return result;
}

Message Message::findById(int messageId) {
   sqlite3* db = Database::getConnection();
   if (!db) return Message();
   
 std::string sql = "SELECT id, sender_id, receiver_id, content, sender_type, receiver_type, is_read, created_at FROM messages WHERE id = ?";
    sqlite3_stmt* stmt;
    Message message;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return message;
    
    sqlite3_bind_int(stmt, 1, messageId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        message.id = sqlite3_column_int(stmt, 0);
        message.senderId = sqlite3_column_int(stmt, 1);
        message.receiverId = sqlite3_column_int(stmt, 2);
        message.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        message.senderType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        message.receiverType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        message.isRead = sqlite3_column_int(stmt, 6) == 1;
        message.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    }
    
    sqlite3_finalize(stmt);
    return message;
}

std::vector<Message> Message::getConversation(int user1Id, int user2Id) {
 sqlite3* db = Database::getConnection();
 if (!db) return std::vector<Message>();
 
   std::vector<Message> messages;
    std::string sql = "SELECT id, sender_id, receiver_id, content, sender_type, receiver_type, is_read, created_at FROM messages WHERE (sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?) ORDER BY created_at ASC";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return messages;
    
    sqlite3_bind_int(stmt, 1, user1Id);
    sqlite3_bind_int(stmt, 2, user2Id);
    sqlite3_bind_int(stmt, 3, user2Id);
    sqlite3_bind_int(stmt, 4, user1Id);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Message message;
        message.id = sqlite3_column_int(stmt, 0);
        message.senderId = sqlite3_column_int(stmt, 1);
        message.receiverId = sqlite3_column_int(stmt, 2);
        message.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        message.senderType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        message.receiverType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        message.isRead = sqlite3_column_int(stmt, 6) == 1;
        message.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        messages.push_back(message);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

std::vector<Message> Message::getInbox(int userId) {
 sqlite3* db = Database::getConnection();
 if (!db) return std::vector<Message>();
 
   std::vector<Message> messages;
    std::string sql = "SELECT id, sender_id, receiver_id, content, sender_type, receiver_type, is_read, created_at FROM messages WHERE receiver_id = ? ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return messages;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Message message;
        message.id = sqlite3_column_int(stmt, 0);
        message.senderId = sqlite3_column_int(stmt, 1);
        message.receiverId = sqlite3_column_int(stmt, 2);
        message.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        message.senderType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        message.receiverType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        message.isRead = sqlite3_column_int(stmt, 6) == 1;
        message.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        messages.push_back(message);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

std::vector<Message> Message::getSent(int userId) {
  sqlite3* db = Database::getConnection();
  if (!db) return std::vector<Message>();
  
  std::vector<Message> messages;
    std::string sql = "SELECT id, sender_id, receiver_id, content, sender_type, receiver_type, is_read, created_at FROM messages WHERE sender_id = ? ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return messages;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Message message;
        message.id = sqlite3_column_int(stmt, 0);
        message.senderId = sqlite3_column_int(stmt, 1);
        message.receiverId = sqlite3_column_int(stmt, 2);
        message.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        message.senderType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        message.receiverType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        message.isRead = sqlite3_column_int(stmt, 6) == 1;
        message.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        messages.push_back(message);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

int Message::getUnreadCount(int userId) {
   sqlite3* db = Database::getConnection();
   if (!db) return 0;
   
 std::string sql = "SELECT COUNT(*) FROM messages WHERE receiver_id = ? AND is_read = 0";
    sqlite3_stmt* stmt;
    int count = 0;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return 0;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}
void Message::setId(int id) {
    this->id = id;
}

void Message::setCreatedAt(const std::string& createdAt) {
    this->createdAt = createdAt;
}
std::string Message::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"senderId\":" << senderId << ",";
    json << "\"receiverId\":" << receiverId << ",";
    json << "\"content\":\"" << content << "\",";
    json << "\"senderType\":\"" << senderType << "\",";
    json << "\"receiverType\":\"" << receiverType << "\",";
    json << "\"isRead\":" << (isRead ? "true" : "false") << ",";
    json << "\"createdAt\":\"" << createdAt << "\"";
    json << "}";
    return json.str();
}

Message Message::fromJson(std::string jsonStr) {
    Message message;
    return message;
}