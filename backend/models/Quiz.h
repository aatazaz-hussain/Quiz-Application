#ifndef QUIZ_H
#define QUIZ_H

#include <string>
#include <vector>

class Question {
private:
    int id;
    int quizId;
    std::string text;
    std::string type;
    int points;
    std::vector<std::string> options;
    int correctOption;

public:
    Question();
    Question(int id, int quizId, std::string text, std::string type, int points,
             std::vector<std::string> options, int correctOption);
    Question(int id, int quizId, std::string text, std::string type, int points);
    
    int getId() const;
    int getQuizId() const;
    std::string getText() const;
    std::string getType() const;
    int getPoints() const;
    std::vector<std::string> getOptions() const;
    int getCorrectOption() const;
    
    void setText(std::string text);
    void setType(std::string type);
    void setPoints(int points);
    void setOptions(std::vector<std::string> options);
    void setCorrectOption(int correctOption);
    
    void addOption(std::string option, bool isCorrect);
    bool save();
    bool update();
    static Question findById(int questionId);
    static std::vector<Question> findByQuizId(int quizId);
    std::string toJson() const;
    static Question fromJson(std::string jsonStr);
};

class Quiz {
private:
    int id;
    std::string title;
    std::string description;
    int teacherId;
    std::string subject;
    std::string status;
    int timeLimit;
    std::vector<Question> questions;

public:
    Quiz();
    Quiz(int id, std::string title, std::string description, int teacherId, std::string subject);
    
    int getId() const;
    std::string getTitle() const;
    std::string getDescription() const;
    int getTeacherId() const;
    std::string getSubject() const;
    std::string getStatus() const;
    int getTimeLimit() const;
    std::vector<Question> getQuestions() const;
    
    void setTitle(std::string title);
    void setDescription(std::string description);
    void setSubject(std::string subject);
    void setTimeLimit(int minutes);
    
    void publish();
    void unpublish();
    
    bool save();
    bool update();
    bool remove();
    static Quiz findById(int quizId);
    static std::vector<Quiz> findByTeacher(int teacherId);
    static std::vector<Quiz> getPublishedQuizzes();
    static std::vector<Quiz> getAvailableForStudent(int studentId);
    
    void addQuestion(Question question);
    bool deleteQuestion(int questionId);
    
    std::string toJson() const;
    static Quiz fromJson(std::string jsonStr);
};

#endif