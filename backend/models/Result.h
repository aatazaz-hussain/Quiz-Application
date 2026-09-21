#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include <ctime>

class Grade {
private:
    int id;
    int resultId;
    int questionId;
    int awardedPoints;
    std::string feedback;

public:
    Grade();
    Grade(int id, int resultId, int questionId, int awardedPoints);
    
    int getId() const;
    int getResultId() const;
    int getQuestionId() const;
    int getAwardedPoints() const;
    std::string getFeedback() const;
    
    void setAwardedPoints(int points);
    void setFeedback(std::string feedback);
    
    bool save();
    static std::vector<Grade> findByResultId(int resultId);
    
    std::string toJson() const;
};

class Submission {
private:
    int id;
    int studentId;
    int quizId;
    std::string answers;
    std::string submittedAt;

public:
    Submission();
    Submission(int id, int studentId, int quizId, std::string answers);
    
    int getId() const;
    int getStudentId() const;
    int getQuizId() const;
    std::string getAnswers() const;
    std::string getSubmittedAt() const;
    
    void setAnswers(std::string answers);
    
    bool save();
    static Submission findByStudentAndQuiz(int studentId, int quizId);
    
    std::string toJson() const;
};

class Result {
private:
    int id;
    int submissionId;
    int studentId;
    int quizId;
    int totalScore;
    int maxScore;
    std::string status;
    std::string gradedAt;
    std::vector<Grade> grades;

public:
    Result();
    Result(int id, int submissionId, int studentId, int quizId, int totalScore, int maxScore);
    static std::vector<Result> findByQuizId(int quizId);
    int getId() const;
    int getSubmissionId() const;
    int getStudentId() const;
    int getQuizId() const;
    int getTotalScore() const;
    int getMaxScore() const;
    std::string getStatus() const;
    float getPercentage() const;
    std::vector<Grade> getGrades() const;
std::string getGradedAt() const;

    void setTotalScore(int score);
    void setStatus(std::string status);
    void addGrade(Grade grade);
    
    bool save();
    bool calculateScore();
    static Result findById(int resultId);
    static std::vector<Result> findByStudent(int studentId);
    static std::vector<Result> findByQuiz(int quizId);
    static Result findBySubmission(int submissionId);
    
    std::string toJson() const;
    static Result fromJson(std::string jsonStr);
};

#endif