#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

class User {
protected:
    int id;
    std::string name;
    std::string email;
    std::string password;
    std::string userType;
    std::string createdAt;

public:
    User();
    User(int id, std::string name, std::string email, std::string userType);
    
    int getId() const;
    std::string getName() const;
    std::string getEmail() const;
    std::string getUserType() const;
    
    void setName(std::string name);
    void setEmail(std::string email);
    void setPassword(std::string password);
    
    bool save();
    bool update();
    bool remove();
    static User findById(int userId);
    static std::vector<User> getAll();
    static User authenticate(std::string email, std::string password);
    
    std::string toJson() const;
    static User fromJson(std::string jsonStr);
};

class Student : public User {
private:
    std::string enrollmentNo;
    
public:
    Student();
    Student(int id, std::string name, std::string email, std::string enrollmentNo);
    
    std::string getEnrollmentNo() const;
    void setEnrollmentNo(std::string enrollmentNo);
    
    std::vector<int> getCompletedQuizzes();
    std::vector<int> getPendingQuizzes();
    bool save();
};

class Teacher : public User {
private:
    std::string employeeId;
    
public:
    Teacher();
    Teacher(int id, std::string name, std::string email, std::string employeeId);
    
    std::string getEmployeeId() const;
    void setEmployeeId(std::string employeeId);
    
    std::vector<int> getCreatedQuizzes();
    std::vector<int> getStudents();
     bool save();
};

class Admin : public User {
public:
    Admin();
    Admin(int id, std::string name, std::string email);
    
    std::vector<User> getAllUsers();
    bool deleteUser(int userId);
    bool updateUserRole(int userId, std::string newRole);
};

#endif