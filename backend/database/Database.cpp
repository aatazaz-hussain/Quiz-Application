#include "Database.h"
#include <iostream>

sqlite3* Database::db = nullptr;
bool Database::isOpen = false;

bool Database::executeSQL(const std::string& sql) {
    char* error = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
    if(rc != SQLITE_OK) {
        if(error) sqlite3_free(error);
        return false;
    }
    return true;
}

bool Database::createTables() {
    const char* tables[] = {
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "user_type TEXT NOT NULL,"
        "enrollment_no TEXT,"
        "employee_id TEXT,"
        "created_at TEXT"
        ");",

        "CREATE TABLE IF NOT EXISTS quizzes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "description TEXT,"
        "teacher_id INTEGER NOT NULL,"
        "subject TEXT,"
        "status TEXT DEFAULT 'draft',"
        "time_limit INTEGER DEFAULT 30,"
        "created_at TEXT,"
        "FOREIGN KEY(teacher_id) REFERENCES users(id)"
        ");",

        "CREATE TABLE IF NOT EXISTS questions ("
   " id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "quiz_id INTEGER NOT NULL,"
   " text TEXT NOT NULL,"
    "type TEXT DEFAULT 'multiple_choice',"
    "points INTEGER DEFAULT 1,"
    "correct_option INTEGER DEFAULT -1,"
    "FOREIGN KEY (quiz_id) REFERENCES quizzes(id) ON DELETE CASCADE"
");"

       "CREATE TABLE IF NOT EXISTS question_options ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "question_id INTEGER NOT NULL,"
    "option_text TEXT NOT NULL,"
    "option_order INTEGER DEFAULT 0,"
    "FOREIGN KEY (question_id) REFERENCES questions(id) ON DELETE CASCADE"
");"

        "CREATE TABLE IF NOT EXISTS submissions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER NOT NULL,"
        "quiz_id INTEGER NOT NULL,"
        "answers TEXT,"
        "submitted_at TEXT,"
        "FOREIGN KEY(student_id) REFERENCES users(id),"
        "FOREIGN KEY(quiz_id) REFERENCES quizzes(id)"
        ");",

        "CREATE TABLE IF NOT EXISTS results ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "submission_id INTEGER NOT NULL,"
        "student_id INTEGER NOT NULL,"
        "quiz_id INTEGER NOT NULL,"
        "total_score INTEGER,"
        "max_score INTEGER,"
        "status TEXT DEFAULT 'pending',"
        "graded_at TEXT,"
        "FOREIGN KEY(submission_id) REFERENCES submissions(id),"
        "FOREIGN KEY(student_id) REFERENCES users(id),"
        "FOREIGN KEY(quiz_id) REFERENCES quizzes(id)"
        ");",

        "CREATE TABLE IF NOT EXISTS grades ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "result_id INTEGER NOT NULL,"
        "question_id INTEGER NOT NULL,"
        "awarded_points INTEGER,"
        "feedback TEXT,"
        "FOREIGN KEY(result_id) REFERENCES results(id),"
        "FOREIGN KEY(question_id) REFERENCES questions(id)"
        ");",

        "CREATE TABLE IF NOT EXISTS notifications ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "content TEXT NOT NULL,"
        "type TEXT DEFAULT 'system',"
        "is_read INTEGER DEFAULT 0,"
        "created_at TEXT,"
        "FOREIGN KEY(user_id) REFERENCES users(id)"
        ");",

        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sender_id INTEGER NOT NULL,"
        "receiver_id INTEGER NOT NULL,"
        "content TEXT NOT NULL,"
        "sender_type TEXT,"
        "receiver_type TEXT,"
        "is_read INTEGER DEFAULT 0,"
        "created_at TEXT,"
        "FOREIGN KEY(sender_id) REFERENCES users(id),"
        "FOREIGN KEY(receiver_id) REFERENCES users(id)"
        ");",

        "CREATE TABLE IF NOT EXISTS sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "token TEXT UNIQUE NOT NULL,"
        "user_type TEXT,"
        "created_at TEXT,"
        "expires_at TEXT,"
        "is_valid INTEGER DEFAULT 1,"
        "FOREIGN KEY(user_id) REFERENCES users(id)"
        ");"
    };

    int tableCount = sizeof(tables) / sizeof(tables[0]);
    for(int i = 0; i < tableCount; i++) {
        if(!executeSQL(tables[i])) {
            return false;
        }
    }
    return true;
}

bool Database::open() {
    if(isOpen) return true;
    
    int rc = sqlite3_open("quiz-system.db", &db);
    if(rc != SQLITE_OK) {
        return false;
    }
    
    isOpen = true;
    return true;
}

void Database::close() {
    if(db) {
        sqlite3_close(db);
        db = nullptr;
    }
    isOpen = false;
}

sqlite3* Database::getConnection() {
    return db;
}

bool Database::userExists(const std::string& email) {
    std::string sql = "SELECT COUNT(*) FROM users WHERE email = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    
    bool exists = false;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::createAdminUser() {
    if(userExists("admin@quizmaster.com")) return true;
    
    std::string sql = "INSERT INTO users (name, email, password, user_type, created_at) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, "Administrator", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, "admin@quizmaster.com", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, "admin123", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, "admin", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, "now", -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

void Database::initialize() {
    if(!open()) return;
    
    createTables();
    createAdminUser();
    
    std::cout << "Database initialized successfully." << std::endl;
}

std::string Database::getError() {
    if(db) {
        return sqlite3_errmsg(db);
    }
    return "Database not connected";
}
