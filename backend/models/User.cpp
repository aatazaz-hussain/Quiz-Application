#include "User.h"
#include "database/Database.h"
#include <sqlite3.h>
#include <sstream>
#include <ctime>
#include <iostream>

User::User() : id(0) {}

User::User(int id, std::string name, std::string email, std::string userType) 
    : id(id), name(name), email(email), userType(userType) {}

int User::getId() const { return id; }
std::string User::getName() const { return name; }
std::string User::getEmail() const { return email; }
std::string User::getUserType() const { return userType; }

void User::setName(std::string name) { this->name = name; }
void User::setEmail(std::string email) { this->email = email; }
void User::setPassword(std::string password) { this->password = password; }

bool User::save() {
    sqlite3* db = Database::getConnection();
    if (!db) {
        std::cout << "ERROR: Database connection null in User::save()" << std::endl;
        return false;
    }
    
    std::string sql = "INSERT INTO users (name, email, password, user_type, created_at) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cout << "ERROR preparing statement in User::save()" << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, userType.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    sqlite3_bind_text(stmt, 5, std::to_string(now).c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    if (result) std::cout << "User saved with ID: " << id << std::endl;
    else std::cout << "Failed to save user" << std::endl;
    
    return result;
}

bool User::update() {
    sqlite3* db = Database::getConnection();
    if (!db) return false;

    std::string sql = "UPDATE users SET name = ?, email = ?, password = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, id);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool User::remove() {
    sqlite3* db = Database::getConnection();
    if (!db) return false;
    
    std::string sql = "DELETE FROM users WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, id);
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

User User::findById(int userId) {
    sqlite3* db = Database::getConnection();
    if (!db) return User();

    std::string sql = "SELECT id, name, email, user_type FROM users WHERE id = ?";
    sqlite3_stmt* stmt;
    User user;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return user;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.userType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }
    
    sqlite3_finalize(stmt);
    return user;
}

std::vector<User> User::getAll() {
    sqlite3* db = Database::getConnection();
    if (!db) return std::vector<User>();

    std::vector<User> users;
    std::string sql = "SELECT id, name, email, user_type FROM users";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return users;
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.userType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        users.push_back(user);
    }
    
    sqlite3_finalize(stmt);
    return users;
}

User User::authenticate(std::string email, std::string password) {
    sqlite3* db = Database::getConnection();
    if (!db) {
        std::cout << "ERROR: Database null in authenticate()" << std::endl;
        return User();
    }
    
    std::string sql = "SELECT id, name, email, user_type FROM users WHERE email = ? AND password = ?";
    sqlite3_stmt* stmt;
    User user;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return user;
    
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.userType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::cout << "Auth success for user: " << user.id << std::endl;
    } else {
        std::cout << "Auth failed for email: " << email << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return user;
}

std::string User::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"name\":\"" << name << "\",";
    json << "\"email\":\"" << email << "\",";
    json << "\"userType\":\"" << userType << "\"";
    json << "}";
    return json.str();
}

User User::fromJson(std::string jsonStr) {
    User user;
    size_t pos;
    
    pos = jsonStr.find("\"id\":");
    if(pos != std::string::npos) {
        user.id = std::stoi(jsonStr.substr(pos + 5));
    }
    
    pos = jsonStr.find("\"name\":\"");
    if(pos != std::string::npos) {
        size_t end = jsonStr.find("\"", pos + 8);
        user.name = jsonStr.substr(pos + 8, end - pos - 8);
    }
    
    pos = jsonStr.find("\"email\":\"");
    if(pos != std::string::npos) {
        size_t end = jsonStr.find("\"", pos + 9);
        user.email = jsonStr.substr(pos + 9, end - pos - 9);
    }
    
    return user;
}

Student::Student() : User() {
    userType = "student";
}

Student::Student(int id, std::string name, std::string email, std::string enrollmentNo) 
    : User(id, name, email, "student"), enrollmentNo(enrollmentNo) {}

std::string Student::getEnrollmentNo() const { return enrollmentNo; }
void Student::setEnrollmentNo(std::string enrollmentNo) { this->enrollmentNo = enrollmentNo; }

bool Student::save() {
    sqlite3* db = Database::getConnection();
    if (!db) {
        std::cout << "ERROR: Database null in Student::save()" << std::endl;
        return false;
    }
    
    std::string sql = "INSERT INTO users (name, email, password, user_type, enrollment_no, created_at) VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cout << "ERROR preparing statement in Student::save()" << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, userType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, enrollmentNo.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    sqlite3_bind_text(stmt, 6, std::to_string(now).c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    if (result) std::cout << "Student saved with ID: " << id << std::endl;
    else std::cout << "Failed to save student" << std::endl;
    
    return result;
}

std::vector<int> Student::getCompletedQuizzes() {
    std::vector<int> quizIds;
    return quizIds;
}

std::vector<int> Student::getPendingQuizzes() {
    std::vector<int> quizIds;
    return quizIds;
}

Teacher::Teacher() : User() {
    userType = "teacher";
}

Teacher::Teacher(int id, std::string name, std::string email, std::string employeeId)
    : User(id, name, email, "teacher"), employeeId(employeeId) {}

std::string Teacher::getEmployeeId() const { return employeeId; }
void Teacher::setEmployeeId(std::string employeeId) { this->employeeId = employeeId; }

bool Teacher::save() {
    sqlite3* db = Database::getConnection();
    if (!db) {
        std::cout << "ERROR: Database null in Teacher::save()" << std::endl;
        return false;
    }
    
    std::string sql = "INSERT INTO users (name, email, password, user_type, employee_id, created_at) VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cout << "ERROR preparing statement in Teacher::save()" << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, userType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, employeeId.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    sqlite3_bind_text(stmt, 6, std::to_string(now).c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    if (result) std::cout << "Teacher saved with ID: " << id << std::endl;
    else std::cout << "Failed to save teacher" << std::endl;
    
    return result;
}

std::vector<int> Teacher::getCreatedQuizzes() {
    std::vector<int> quizIds;
    return quizIds;
}

std::vector<int> Teacher::getStudents() {
    std::vector<int> studentIds;
    return studentIds;
}

Admin::Admin() : User() {
    userType = "admin";
}

Admin::Admin(int id, std::string name, std::string email)
    : User(id, name, email, "admin") {}

std::vector<User> Admin::getAllUsers() {
    return User::getAll();
}

bool Admin::deleteUser(int userId) {
    sqlite3* db = Database::getConnection();
    if (!db) return false;

    std::string sql = "DELETE FROM users WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool Admin::updateUserRole(int userId, std::string newRole) {
    sqlite3* db = Database::getConnection();
    if (!db) return false;
    
    std::string sql = "UPDATE users SET user_type = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, newRole.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userId);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}