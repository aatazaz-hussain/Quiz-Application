#include "Session.h"
#include <sqlite3.h>
#include <sstream>
#include "database/Database.h"

Session::Session() : id(0), userId(0), isValid(true) {}

Session::Session(int id, int userId, std::string token, std::string userType)
    : id(id), userId(userId), token(token), userType(userType), isValid(true) {}

int Session::getId() const { return id; }
int Session::getUserId() const { return userId; }
std::string Session::getToken() const { return token; }
std::string Session::getUserType() const { return userType; }
std::string Session::getCreatedAt() const { return createdAt; }
std::string Session::getExpiresAt() const { return expiresAt; }
bool Session::getIsValid() const { return isValid; }

void Session::setUserId(int userId) { this->userId = userId; }
void Session::setToken(std::string token) { this->token = token; }
void Session::setUserType(std::string userType) { this->userType = userType; }

void Session::invalidate() { isValid = false; }

bool Session::isExpired() const {
    if(expiresAt.empty()) return false;
    time_t now = time(0);
    time_t expireTime = std::stol(expiresAt);
    return now > expireTime;
}

bool Session::save() {
   sqlite3* db = Database::getConnection();
 std::string sql = "INSERT INTO sessions (user_id, token, user_type, created_at, expires_at, is_valid) VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, userType.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    createdAt = std::to_string(now);
    sqlite3_bind_text(stmt, 4, createdAt.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t expire = now + (24 * 60 * 60);
    expiresAt = std::to_string(expire);
    sqlite3_bind_text(stmt, 5, expiresAt.c_str(), -1, SQLITE_TRANSIENT);
    
    sqlite3_bind_int(stmt, 6, isValid ? 1 : 0);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return result;
}

bool Session::update() {
  sqlite3* db = Database::getConnection();
  std::string sql = "UPDATE sessions SET user_id = ?, token = ?, user_type = ?, expires_at = ?, is_valid = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, userType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, expiresAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, isValid ? 1 : 0);
    sqlite3_bind_int(stmt, 6, id);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

Session Session::findByToken(std::string token) {
  sqlite3* db = Database::getConnection();
  std::string sql = "SELECT id, user_id, token, user_type, created_at, expires_at, is_valid FROM sessions WHERE token = ? AND is_valid = 1";
    sqlite3_stmt* stmt;
    Session session;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return session;
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        session.id = sqlite3_column_int(stmt, 0);
        session.userId = sqlite3_column_int(stmt, 1);
        session.token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.userType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        session.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        session.expiresAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        session.isValid = sqlite3_column_int(stmt, 6) == 1;
    }
    
    sqlite3_finalize(stmt);
    return session;
}

Session Session::findByUserId(int userId) {
  sqlite3* db = Database::getConnection();
  std::string sql = "SELECT id, user_id, token, user_type, created_at, expires_at, is_valid FROM sessions WHERE user_id = ? AND is_valid = 1 ORDER BY created_at DESC LIMIT 1";
    sqlite3_stmt* stmt;
    Session session;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return session;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        session.id = sqlite3_column_int(stmt, 0);
        session.userId = sqlite3_column_int(stmt, 1);
        session.token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.userType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        session.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        session.expiresAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        session.isValid = sqlite3_column_int(stmt, 6) == 1;
    }
    
    sqlite3_finalize(stmt);
    return session;
}

bool Session::deleteByToken(std::string token) {
  sqlite3* db = Database::getConnection();
  std::string sql = "DELETE FROM sessions WHERE token = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool Session::deleteByUserId(int userId) {
  sqlite3* db = Database::getConnection();
  std::string sql = "DELETE FROM sessions WHERE user_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

std::string Session::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"userId\":" << userId << ",";
    json << "\"token\":\"" << token << "\",";
    json << "\"userType\":\"" << userType << "\",";
    json << "\"createdAt\":\"" << createdAt << "\",";
    json << "\"expiresAt\":\"" << expiresAt << "\",";
    json << "\"isValid\":" << (isValid ? "true" : "false");
    json << "}";
    return json.str();
}

Session Session::fromJson(std::string jsonStr) {
    Session session;
    return session;
}