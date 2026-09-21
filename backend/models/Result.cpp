#include "Result.h"
#include <sqlite3.h>
#include <sstream>
#include "database/Database.h"

Grade::Grade() : id(0), resultId(0), questionId(0), awardedPoints(0) {}

Grade::Grade(int id, int resultId, int questionId, int awardedPoints)
    : id(id), resultId(resultId), questionId(questionId), awardedPoints(awardedPoints) {}

int Grade::getId() const { return id; }
int Grade::getResultId() const { return resultId; }
int Grade::getQuestionId() const { return questionId; }
int Grade::getAwardedPoints() const { return awardedPoints; }
std::string Grade::getFeedback() const { return feedback; }

void Grade::setAwardedPoints(int points) { awardedPoints = points; }
void Grade::setFeedback(std::string feedback) { this->feedback = feedback; }

bool Grade::save() {
   sqlite3* db = Database::getConnection();
 std::string sql = "INSERT INTO grades (result_id, question_id, awarded_points, feedback) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, resultId);
    sqlite3_bind_int(stmt, 2, questionId);
    sqlite3_bind_int(stmt, 3, awardedPoints);
    sqlite3_bind_text(stmt, 4, feedback.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return result;
}

std::vector<Grade> Grade::findByResultId(int resultId) {
   sqlite3* db = Database::getConnection();
 std::vector<Grade> grades;
    std::string sql = "SELECT id, result_id, question_id, awarded_points, feedback FROM grades WHERE result_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return grades;
    
    sqlite3_bind_int(stmt, 1, resultId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Grade grade;
        grade.id = sqlite3_column_int(stmt, 0);
        grade.resultId = sqlite3_column_int(stmt, 1);
        grade.questionId = sqlite3_column_int(stmt, 2);
        grade.awardedPoints = sqlite3_column_int(stmt, 3);
        grade.feedback = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        grades.push_back(grade);
    }
    
    sqlite3_finalize(stmt);
    return grades;
}

std::string Grade::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"resultId\":" << resultId << ",";
    json << "\"questionId\":" << questionId << ",";
    json << "\"awardedPoints\":" << awardedPoints << ",";
    json << "\"feedback\":\"" << feedback << "\"";
    json << "}";
    return json.str();
}

Submission::Submission() : id(0), studentId(0), quizId(0) {}

Submission::Submission(int id, int studentId, int quizId, std::string answers)
    : id(id), studentId(studentId), quizId(quizId), answers(answers) {}

int Submission::getId() const { return id; }
int Submission::getStudentId() const { return studentId; }
int Submission::getQuizId() const { return quizId; }
std::string Submission::getAnswers() const { return answers; }
std::string Submission::getSubmittedAt() const { return submittedAt; }

void Submission::setAnswers(std::string answers) { this->answers = answers; }

bool Submission::save() {
   sqlite3* db = Database::getConnection();
 std::string sql = "INSERT INTO submissions (student_id, quiz_id, answers, submitted_at) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, studentId);
    sqlite3_bind_int(stmt, 2, quizId);
    sqlite3_bind_text(stmt, 3, answers.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    submittedAt = std::to_string(now);
    sqlite3_bind_text(stmt, 4, submittedAt.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return result;
}

Submission Submission::findByStudentAndQuiz(int studentId, int quizId) {
  sqlite3* db = Database::getConnection();
  std::string sql = "SELECT id, student_id, quiz_id, answers, submitted_at FROM submissions WHERE student_id = ? AND quiz_id = ?";
    sqlite3_stmt* stmt;
    Submission submission;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return submission;
    
    sqlite3_bind_int(stmt, 1, studentId);
    sqlite3_bind_int(stmt, 2, quizId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        submission.id = sqlite3_column_int(stmt, 0);
        submission.studentId = sqlite3_column_int(stmt, 1);
        submission.quizId = sqlite3_column_int(stmt, 2);
        submission.answers = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        submission.submittedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }
    
    sqlite3_finalize(stmt);
    return submission;
}

std::string Submission::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"studentId\":" << studentId << ",";
    json << "\"quizId\":" << quizId << ",";
    json << "\"answers\":\"" << answers << "\",";
    json << "\"submittedAt\":\"" << submittedAt << "\"";
    json << "}";
    return json.str();
}

Result::Result() : id(0), submissionId(0), studentId(0), quizId(0), totalScore(0), maxScore(100), status("pending") {}

Result::Result(int id, int submissionId, int studentId, int quizId, int totalScore, int maxScore)
    : id(id), submissionId(submissionId), studentId(studentId), quizId(quizId), totalScore(totalScore), maxScore(maxScore), status("graded") {}

int Result::getId() const { return id; }
int Result::getSubmissionId() const { return submissionId; }
int Result::getStudentId() const { return studentId; }
int Result::getQuizId() const { return quizId; }
int Result::getTotalScore() const { return totalScore; }
int Result::getMaxScore() const { return maxScore; }
std::string Result::getStatus() const { return status; }
float Result::getPercentage() const { 
    if(maxScore == 0) return 0.0f;
    return (totalScore * 100.0f) / maxScore; 
}
std::vector<Grade> Result::getGrades() const { return grades; }

void Result::setTotalScore(int score) { totalScore = score; }
void Result::setStatus(std::string status) { this->status = status; }
void Result::addGrade(Grade grade) { grades.push_back(grade); }

bool Result::save() {
   sqlite3* db = Database::getConnection();
 std::string sql = "INSERT INTO results (submission_id, student_id, quiz_id, total_score, max_score, status, graded_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, submissionId);
    sqlite3_bind_int(stmt, 2, studentId);
    sqlite3_bind_int(stmt, 3, quizId);
    sqlite3_bind_int(stmt, 4, totalScore);
    sqlite3_bind_int(stmt, 5, maxScore);
    sqlite3_bind_text(stmt, 6, status.c_str(), -1, SQLITE_TRANSIENT);
    
    time_t now = time(0);
    gradedAt = std::to_string(now);
    sqlite3_bind_text(stmt, 7, gradedAt.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = sqlite3_step(stmt) == SQLITE_DONE;
    id = sqlite3_last_insert_rowid(db);
    
    for(Grade& grade : grades) {
        grade.save();
    }
    
    sqlite3_finalize(stmt);
    return result;
}

bool Result::calculateScore() {
    int score = 0;
    for(const Grade& grade : grades) {
        score += grade.getAwardedPoints();
    }
    totalScore = score;
    status = "graded";
    return true;
}

Result Result::findById(int resultId) {
  sqlite3* db = Database::getConnection();
  std::string sql = "SELECT id, submission_id, student_id, quiz_id, total_score, max_score, status FROM results WHERE id = ?";
    sqlite3_stmt* stmt;
    Result result;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return result;
    
    sqlite3_bind_int(stmt, 1, resultId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        result.id = sqlite3_column_int(stmt, 0);
        result.submissionId = sqlite3_column_int(stmt, 1);
        result.studentId = sqlite3_column_int(stmt, 2);
        result.quizId = sqlite3_column_int(stmt, 3);
        result.totalScore = sqlite3_column_int(stmt, 4);
        result.maxScore = sqlite3_column_int(stmt, 5);
        result.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Result> Result::findByStudent(int studentId) {
  sqlite3* db = Database::getConnection();
  std::vector<Result> results;
    std::string sql = "SELECT id, submission_id, student_id, quiz_id, total_score, max_score, status FROM results WHERE student_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return results;
    
    sqlite3_bind_int(stmt, 1, studentId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Result result;
        result.id = sqlite3_column_int(stmt, 0);
        result.submissionId = sqlite3_column_int(stmt, 1);
        result.studentId = sqlite3_column_int(stmt, 2);
        result.quizId = sqlite3_column_int(stmt, 3);
        result.totalScore = sqlite3_column_int(stmt, 4);
        result.maxScore = sqlite3_column_int(stmt, 5);
        result.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        results.push_back(result);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

std::vector<Result> Result::findByQuiz(int quizId) {
  sqlite3* db = Database::getConnection();
  std::vector<Result> results;
    std::string sql = "SELECT id, submission_id, student_id, quiz_id, total_score, max_score, status FROM results WHERE quiz_id = ?";
    sqlite3_stmt* stmt;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return results;
    
    sqlite3_bind_int(stmt, 1, quizId);
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Result result;
        result.id = sqlite3_column_int(stmt, 0);
        result.submissionId = sqlite3_column_int(stmt, 1);
        result.studentId = sqlite3_column_int(stmt, 2);
        result.quizId = sqlite3_column_int(stmt, 3);
        result.totalScore = sqlite3_column_int(stmt, 4);
        result.maxScore = sqlite3_column_int(stmt, 5);
        result.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        results.push_back(result);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

Result Result::findBySubmission(int submissionId) {
   sqlite3* db = Database::getConnection();
 std::string sql = "SELECT id, submission_id, student_id, quiz_id, total_score, max_score, status FROM results WHERE submission_id = ?";
    sqlite3_stmt* stmt;
    Result result;
    
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) return result;
    
    sqlite3_bind_int(stmt, 1, submissionId);
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        result.id = sqlite3_column_int(stmt, 0);
        result.submissionId = sqlite3_column_int(stmt, 1);
        result.studentId = sqlite3_column_int(stmt, 2);
        result.quizId = sqlite3_column_int(stmt, 3);
        result.totalScore = sqlite3_column_int(stmt, 4);
        result.maxScore = sqlite3_column_int(stmt, 5);
        result.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    }
    
    sqlite3_finalize(stmt);
    return result;
}
std::vector<Result> Result::findByQuizId(int quizId) {
    std::vector<Result> list;
    // TODO: implement database lookup
    return list;
}
std::string Result::getGradedAt() const {
    return gradedAt;
}


std::string Result::toJson() const {
    std::stringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"submissionId\":" << submissionId << ",";
    json << "\"studentId\":" << studentId << ",";
    json << "\"quizId\":" << quizId << ",";
    json << "\"totalScore\":" << totalScore << ",";
    json << "\"maxScore\":" << maxScore << ",";
    json << "\"status\":\"" << status << "\",";
    json << "\"percentage\":" << getPercentage() << ",";
    json << "\"grades\":[";
    for(size_t i = 0; i < grades.size(); i++) {
        if(i > 0) json << ",";
        json << grades[i].toJson();
    }
    json << "]";
    json << "}";
    return json.str();
}

Result Result::fromJson(std::string jsonStr) {
    Result result;
    return result;
}