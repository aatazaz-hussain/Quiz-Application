#include "Quiz.h"
#include <sqlite3.h>
#include <sstream>
#include <cstring>
#include <ctime>
#include "database/Database.h"

// Question class implementation
Question::Question() : id(0), quizId(0), points(0), correctOption(-1) {}

Question::Question(int id, int quizId, std::string text, std::string type, int points,
                   std::vector<std::string> options, int correctOption)
    : id(id), quizId(quizId), text(text), type(type), points(points), 
      options(options), correctOption(correctOption) {}

Question::Question(int id, int quizId, std::string text, std::string type, int points)
    : id(id), quizId(quizId), text(text), type(type), points(points), correctOption(-1) {}

int Question::getQuizId() const {
    return quizId;
}

int Question::getId() const { return id; }
std::string Question::getText() const { return text; }
std::string Question::getType() const { return type; }
int Question::getPoints() const { return points; }
std::vector<std::string> Question::getOptions() const { return options; }
int Question::getCorrectOption() const { return correctOption; }

void Question::setText(std::string text) { this->text = text; }
void Question::setType(std::string type) { this->type = type; }
void Question::setPoints(int points) { this->points = points; }
void Question::setOptions(std::vector<std::string> options) { this->options = options; }
void Question::setCorrectOption(int correctOption) { this->correctOption = correctOption; }

void Question::addOption(std::string option, bool isCorrect) {
    options.push_back(option);
    if(isCorrect) {
        correctOption = options.size() - 1;
    }
}

bool Question::save() {
    sqlite3* db = Database::getConnection();
    
    std::string sql = "INSERT INTO questions (quiz_id, text, type, points, correct_option) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, quizId);
    sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, points);
    sqlite3_bind_int(stmt, 5, correctOption);
    
    if(sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    // Save options
    for(size_t i = 0; i < options.size(); i++) {
        std::string optionSql = "INSERT INTO question_options (question_id, option_text, option_order) VALUES (?, ?, ?)";
        sqlite3_stmt* optionStmt;
        
        if(sqlite3_prepare_v2(db, optionSql.c_str(), -1, &optionStmt, NULL) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_int(optionStmt, 1, id);
        sqlite3_bind_text(optionStmt, 2, options[i].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(optionStmt, 3, i);
        
        if(sqlite3_step(optionStmt) != SQLITE_DONE) {
            sqlite3_finalize(optionStmt);
            return false;
        }
        
        sqlite3_finalize(optionStmt);
    }
    
    return true;
}

bool Question::update() {
    sqlite3* db = Database::getConnection();
    
    std::string sql = "UPDATE questions SET text = ?, type = ?, points = ?, correct_option = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, points);
    sqlite3_bind_int(stmt, 4, correctOption);
    sqlite3_bind_int(stmt, 5, id);
    
    if(sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    
    sqlite3_finalize(stmt);
    
    std::string deleteSql = "DELETE FROM question_options WHERE question_id = ?";
    sqlite3_stmt* deleteStmt;
    
    if(sqlite3_prepare_v2(db, deleteSql.c_str(), -1, &deleteStmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(deleteStmt, 1, id);
    sqlite3_step(deleteStmt);
    sqlite3_finalize(deleteStmt);
    
    for(size_t i = 0; i < options.size(); i++) {
        std::string optionSql = "INSERT INTO question_options (question_id, option_text, option_order) VALUES (?, ?, ?)";
        sqlite3_stmt* optionStmt;
        
        if(sqlite3_prepare_v2(db, optionSql.c_str(), -1, &optionStmt, NULL) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_int(optionStmt, 1, id);
        sqlite3_bind_text(optionStmt, 2, options[i].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(optionStmt, 3, i);
        
        if(sqlite3_step(optionStmt) != SQLITE_DONE) {
            sqlite3_finalize(optionStmt);
            return false;
        }
        
        sqlite3_finalize(optionStmt);
    }
    
    return true;
}

Question Question::findById(int questionId) {
    sqlite3* db = Database::getConnection();
    std::string sql = "SELECT id, quiz_id, text, type, points, correct_option FROM questions WHERE id = ?";
    sqlite3_stmt* stmt;
    Question question;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return question;
    
    sqlite3_bind_int(stmt, 1, questionId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        question.id = sqlite3_column_int(stmt, 0);
        question.quizId = sqlite3_column_int(stmt, 1);
        question.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        question.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        question.points = sqlite3_column_int(stmt, 4);
        question.correctOption = sqlite3_column_int(stmt, 5);
        
        std::string optionSql = "SELECT option_text FROM question_options WHERE question_id = ? ORDER BY option_order";
        sqlite3_stmt* optionStmt;
        
        if(sqlite3_prepare_v2(db, optionSql.c_str(), -1, &optionStmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(optionStmt, 1, questionId);
            
            while(sqlite3_step(optionStmt) == SQLITE_ROW) {
                const char* optionText = reinterpret_cast<const char*>(sqlite3_column_text(optionStmt, 0));
                if(optionText) {
                    question.options.push_back(std::string(optionText));
                }
            }
            
            sqlite3_finalize(optionStmt);
        }
    }
    
    sqlite3_finalize(stmt);
    return question;
}

std::vector<Question> Question::findByQuizId(int quizId) {
    sqlite3* db = Database::getConnection();
    std::vector<Question> questions;
    std::string sql = "SELECT id, quiz_id, text, type, points, correct_option FROM questions WHERE quiz_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return questions;
    
    sqlite3_bind_int(stmt, 1, quizId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Question question;
        question.id = sqlite3_column_int(stmt, 0);
        question.quizId = sqlite3_column_int(stmt, 1);
        question.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        question.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        question.points = sqlite3_column_int(stmt, 4);
        question.correctOption = sqlite3_column_int(stmt, 5);
        
        std::string optionSql = "SELECT option_text FROM question_options WHERE question_id = ? ORDER BY option_order";
        sqlite3_stmt* optionStmt;
        
        if(sqlite3_prepare_v2(db, optionSql.c_str(), -1, &optionStmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(optionStmt, 1, question.id);
            
            while(sqlite3_step(optionStmt) == SQLITE_ROW) {
                const char* optionText = reinterpret_cast<const char*>(sqlite3_column_text(optionStmt, 0));
                if(optionText) {
                    question.options.push_back(std::string(optionText));
                }
            }
            
            sqlite3_finalize(optionStmt);
        }
        
        questions.push_back(question);
    }
    
    sqlite3_finalize(stmt);
    return questions;
}

std::string Question::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"quizId\":" << quizId << ",";
    json << "\"text\":\"" << text << "\",";
    json << "\"type\":\"" << type << "\",";
    json << "\"points\":" << points << ",";
    json << "\"options\":[";
    for(size_t i = 0; i < options.size(); i++) {
        if(i > 0) json << ",";
        
        std::string escapedOption = options[i];
        size_t pos = 0;
        while((pos = escapedOption.find('"', pos)) != std::string::npos) {
            escapedOption.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        json << "\"" << escapedOption << "\"";
    }
    json << "],";
    json << "\"correctOption\":" << correctOption;
    json << "}";
    return json.str();
}

Question Question::fromJson(std::string jsonStr) {
    Question question;
    return question;
}

// Quiz class implementation
Quiz::Quiz() : id(0), teacherId(0), timeLimit(30), status("draft") {}

Quiz::Quiz(int id, std::string title, std::string description, int teacherId, std::string subject)
    : id(id), title(title), description(description), teacherId(teacherId), 
      subject(subject), status("draft"), timeLimit(30) {}

int Quiz::getId() const { return id; }
std::string Quiz::getTitle() const { return title; }
std::string Quiz::getDescription() const { return description; }
int Quiz::getTeacherId() const { return teacherId; }
std::string Quiz::getSubject() const { return subject; }
std::string Quiz::getStatus() const { return status; }
int Quiz::getTimeLimit() const { return timeLimit; }
std::vector<Question> Quiz::getQuestions() const { return questions; }

void Quiz::setTitle(std::string title) { this->title = title; }
void Quiz::setDescription(std::string description) { this->description = description; }
void Quiz::setSubject(std::string subject) { this->subject = subject; }
void Quiz::setTimeLimit(int minutes) { this->timeLimit = minutes; }

void Quiz::publish() { status = "published"; }
void Quiz::unpublish() { status = "draft"; }

bool Quiz::save() {
    sqlite3* db = Database::getConnection();
    std::string sql = "INSERT INTO quizzes (title, description, teacher_id, subject, status, time_limit, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, teacherId);
    sqlite3_bind_text(stmt, 4, subject.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, timeLimit);
    
    time_t now = time(0);
    std::string nowStr = std::to_string(now);
    sqlite3_bind_text(stmt, 7, nowStr.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return result;
}

bool Quiz::update() {
    sqlite3* db = Database::getConnection();
    std::string sql = "UPDATE quizzes SET title = ?, description = ?, subject = ?, status = ?, time_limit = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, subject.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, timeLimit);
    sqlite3_bind_int(stmt, 6, id);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

bool Quiz::remove() {
    sqlite3* db = Database::getConnection();
    std::string sql = "DELETE FROM quizzes WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, id);
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

Quiz Quiz::findById(int quizId) {
    sqlite3* db = Database::getConnection();
    std::string sql = "SELECT id, title, description, teacher_id, subject, status, time_limit FROM quizzes WHERE id = ?";
    sqlite3_stmt* stmt;
    Quiz quiz;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return quiz;
    
    sqlite3_bind_int(stmt, 1, quizId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        quiz.id = sqlite3_column_int(stmt, 0);
        quiz.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        quiz.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        quiz.teacherId = sqlite3_column_int(stmt, 3);
        quiz.subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        quiz.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        quiz.timeLimit = sqlite3_column_int(stmt, 6);
    }
    
    sqlite3_finalize(stmt);
    return quiz;
}

std::vector<Quiz> Quiz::findByTeacher(int teacherId) {
    sqlite3* db = Database::getConnection();
    std::vector<Quiz> quizzes;
    std::string sql = "SELECT id, title, description, teacher_id, subject, status, time_limit FROM quizzes WHERE teacher_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return quizzes;
    
    sqlite3_bind_int(stmt, 1, teacherId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Quiz quiz;
        quiz.id = sqlite3_column_int(stmt, 0);
        quiz.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        quiz.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        quiz.teacherId = sqlite3_column_int(stmt, 3);
        quiz.subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        quiz.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        quiz.timeLimit = sqlite3_column_int(stmt, 6);
        quizzes.push_back(quiz);
    }
    
    sqlite3_finalize(stmt);
    return quizzes;
}

std::vector<Quiz> Quiz::getPublishedQuizzes() {
    sqlite3* db = Database::getConnection();
    std::vector<Quiz> quizzes;
    std::string sql = "SELECT id, title, description, teacher_id, subject, status, time_limit FROM quizzes WHERE status = 'published'";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return quizzes;
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Quiz quiz;
        quiz.id = sqlite3_column_int(stmt, 0);
        quiz.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        quiz.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        quiz.teacherId = sqlite3_column_int(stmt, 3);
        quiz.subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        quiz.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        quiz.timeLimit = sqlite3_column_int(stmt, 6);
        quizzes.push_back(quiz);
    }
    
    sqlite3_finalize(stmt);
    return quizzes;
}

std::vector<Quiz> Quiz::getAvailableForStudent(int studentId) {
    return getPublishedQuizzes();
}

void Quiz::addQuestion(Question question) {
    questions.push_back(question);
}

bool Quiz::deleteQuestion(int questionId) {
    sqlite3* db = Database::getConnection();
    std::string sql = "DELETE FROM questions WHERE id = ? AND quiz_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, questionId);
    sqlite3_bind_int(stmt, 2, id);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return result;
}

std::string Quiz::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"title\":\"" << title << "\",";
    json << "\"description\":\"" << description << "\",";
    json << "\"teacherId\":" << teacherId << ",";
    json << "\"subject\":\"" << subject << "\",";
    json << "\"status\":\"" << status << "\",";
    json << "\"timeLimit\":" << timeLimit;
    json << "}";
    return json.str();
}

Quiz Quiz::fromJson(std::string jsonStr) {
    Quiz quiz;
    return quiz;
}