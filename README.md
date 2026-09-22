# 🧠 QuizMaster — Online Quiz Management System

<p align="center">
  <b>A full-stack C++17 quiz management platform for students, teachers, and administrators.</b><br>
  Create quizzes, manage questions, publish assessments, submit answers, calculate results, communicate with users, and manage notifications through a lightweight custom HTTP server and SQLite database.
</p>

<p align="center">

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![SQLite](https://img.shields.io/badge/Database-SQLite-003B57?style=for-the-badge&logo=sqlite&logoColor=white)
![CMake](https://img.shields.io/badge/Build-CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![HTML5](https://img.shields.io/badge/Frontend-HTML5-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-Vanilla-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)
![Status](https://img.shields.io/badge/Status-Academic%20Project-8A2BE2?style=for-the-badge)

</p>

---

## 📌 Overview

**QuizMaster** is a web-based quiz application developed in **C++17** with a custom HTTP server and **SQLite** database.

The application provides separate experiences for:

- 👨‍🎓 **Students**
- 👨‍🏫 **Teachers**
- 🛡️ **Administrators**

Teachers can create and manage quizzes, add questions and answer options, publish assessments, communicate with students, and review results.

Students can view published quizzes, attempt assessments, submit answers, and receive calculated scores/results.

Administrators can manage users and user roles.

The project also includes:

- 🔐 Token-based sessions
- 📝 Quiz creation and publishing
- ❓ Question and option management
- 📊 Automatic quiz grading
- 🏆 Results and scoring
- 💬 Teacher/student messaging
- 🔔 Notifications
- 👥 User management
- 🗄️ SQLite persistence
- 🌐 Custom C++ HTTP routing/server
- 🖥️ HTML/CSS/JavaScript dashboards

---

# ✨ Key Features

| Feature | Description |
|---|---|
| 🔐 Authentication | Login and registration for supported user roles |
| 👨‍🎓 Student Portal | Browse and attempt available quizzes |
| 👨‍🏫 Teacher Portal | Create, manage, publish and review quizzes |
| 🛡️ Admin Portal | Manage users and update roles |
| 📝 Quiz Builder | Create quizzes with descriptions, subjects and time limits |
| ❓ Question Management | Add questions, options, points and correct answers |
| 📢 Publishing | Teachers can publish/unpublish quizzes |
| 🧮 Automatic Grading | Answers are evaluated automatically |
| 📊 Results | Scores, percentages and grading records |
| 💬 Messaging | User-to-user conversations and teacher broadcasts |
| 🔔 Notifications | Create, retrieve and mark notifications as read |
| 🔑 Sessions | Token-based authenticated sessions |
| 🗄️ SQLite | Persistent local database |
| 🌐 Custom HTTP Server | C++ server handles API and static frontend files |

---

# 👥 User Roles

## 👨‍🎓 Student

Students can:

- Register
- Log in
- View available published quizzes
- Open quiz questions
- Select answers
- Submit quizzes
- Receive automatically calculated scores
- View quiz-related results
- Communicate through messages
- Receive notifications

### Student workflow

```text
Login / Register
       │
       ▼
Student Dashboard
       │
       ├── Available Quizzes
       │       │
       │       └── Attempt Quiz
       │               │
       │               ▼
       │          Submit Answers
       │               │
       │               ▼
       │          Automatic Grading
       │               │
       │               ▼
       │             Result
       │
       ├── Messages
       └── Notifications
```

---

## 👨‍🏫 Teacher

Teachers can:

- Register
- Log in
- Create quizzes
- Add questions
- Add answer options
- Define correct answers
- Assign points
- Publish/unpublish quizzes
- View quiz statistics
- View student results
- Message students
- Send broadcast messages
- Receive notifications

### Teacher workflow

```text
Login
  │
  ▼
Teacher Dashboard
  │
  ├── Create Quiz
  │      │
  │      ├── Title
  │      ├── Description
  │      ├── Subject
  │      └── Questions
  │             ├── Options
  │             ├── Correct Answer
  │             └── Points
  │
  ├── Publish Quiz
  │
  ├── View Results
  │
  ├── Messages
  │
  └── Notifications
```

---

## 🛡️ Administrator

The administrator functionality includes:

- View users
- Delete users
- Update user roles
- Manage the overall user base

The application creates a default administrator account during database initialization.

---

# 🏗️ System Architecture

The application follows a lightweight layered architecture:

```text
┌─────────────────────────────────────────────┐
│                 FRONTEND                    │
│ HTML + CSS + JavaScript + Font Awesome     │
│                                             │
│ Login / Student / Teacher / Admin Dashboards│
└──────────────────────┬──────────────────────┘
                       │
                       │ HTTP Requests
                       │ JSON / Query Parameters
                       ▼
┌─────────────────────────────────────────────┐
│              C++ HTTP SERVER                │
│                                             │
│ Server → Router → Request Handlers          │
│ Authentication / Sessions / API Routes      │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│              DOMAIN MODELS                  │
│                                             │
│ User / Student / Teacher / Admin            │
│ Quiz / Question / Result / Submission       │
│ Session / Message / Notification            │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│              DATABASE LAYER                 │
│                                             │
│ SQLite + Prepared Statements                │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
                 quiz-system.db
```

---

# 🧩 Technology Stack

## Backend

- **C++17**
- Custom HTTP server
- Custom routing layer
- SQLite3
- Object-oriented programming
- REST-style API endpoints
- Token-based sessions

## Frontend

- HTML5
- CSS3
- Vanilla JavaScript
- Font Awesome 6.4.0
- Browser `localStorage`
- Fetch API

## Database

- SQLite
- Foreign-key relationships
- Prepared SQL statements
- Persistent local database

## Build System

- CMake
- Make-compatible build files
- C++17 standard

---

# 📁 Project Structure

```text
Quiz Application/
│
├── backend/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   │
│   ├── database/
│   │   ├── Database.h
│   │   └── Database.cpp
│   │
│   ├── models/
│   │   ├── User.h
│   │   ├── User.cpp
│   │   ├── Quiz.h
│   │   ├── Quiz.cpp
│   │   ├── Result.h
│   │   ├── Result.cpp
│   │   ├── Session.h
│   │   ├── Session.cpp
│   │   ├── Message.h
│   │   └── Message.cpp
│   │
│   ├── server/
│   │   ├── Server.h
│   │   ├── Server.cpp
│   │   ├── Router.h
│   │   ├── Router.cpp
│   │   ├── RequestHandler.h
│   │   └── RequestHandler.cpp
│   │
│   ├── utils/
│   │   ├── Json.h
│   │   └── Json.cpp
│   │
│   ├── sqlite3.c
│   └── sqlite3.h
│
├── Frontend/
│   ├── index.html
│   ├── login.html
│   ├── admin-dashboard.html
│   ├── teacher-dashboard.html
│   └── student-dashboard.html
│
├── build/
│   ├── quiz-system.db
│   ├── sqlite3.dll
│   └── CMake-generated build files
│
└── .vscode/
    ├── settings.json
    └── c_cpp_properties.json
```

> **Note:** The supplied project includes generated `build/` artifacts. For a clean GitHub repository, generated build output is normally excluded with `.gitignore`.

---

# 🗄️ Database Design

The system uses SQLite and initializes the database automatically.

Database file:

```text
quiz-system.db
```

The application creates the following main tables.

## `users`

Stores all application users.

Important fields:

```text
id
name
email
password
user_type
enrollment_no
employee_id
created_at
```

`user_type` distinguishes:

```text
admin
teacher
student
```

---

## `quizzes`

Stores teacher-created quizzes.

Fields include:

```text
id
title
description
teacher_id
subject
status
time_limit
created_at
```

Quiz status can include:

```text
draft
published
```

---

## `questions`

Stores individual quiz questions.

Fields:

```text
id
quiz_id
text
type
points
correct_option
```

---

## `question_options`

Stores answer options for each question.

Fields:

```text
id
question_id
option_text
option_order
```

---

## `submissions`

Stores student quiz submissions.

Fields:

```text
id
student_id
quiz_id
answers
submitted_at
```

---

## `results`

Stores calculated quiz results.

Fields:

```text
id
submission_id
student_id
quiz_id
total_score
max_score
status
graded_at
```

---

## `grades`

Stores question-level grading information.

Fields:

```text
id
result_id
question_id
awarded_points
feedback
```

---

## `notifications`

Stores user notifications.

Fields:

```text
id
user_id
content
type
is_read
created_at
```

---

## `messages`

Stores communication between users.

Fields:

```text
id
sender_id
receiver_id
content
sender_type
receiver_type
is_read
created_at
```

---

## `sessions`

Stores authenticated login sessions.

Fields:

```text
id
user_id
token
user_type
created_at
expires_at
is_valid
```

---

# 🔗 Database Relationships

```text
                    ┌──────────────┐
                    │    USERS     │
                    └──────┬───────┘
                           │
             ┌─────────────┼──────────────┐
             │             │              │
             ▼             ▼              ▼
         TEACHERS      STUDENTS        SESSIONS
             │             │
             │             │
             ▼             │
          QUIZZES ◄────────┘
             │
             ▼
        QUESTIONS
             │
             ▼
     QUESTION_OPTIONS

STUDENT + QUIZ
      │
      ▼
 SUBMISSIONS
      │
      ▼
   RESULTS
      │
      ▼
    GRADES

USERS ─────► MESSAGES
USERS ─────► NOTIFICATIONS
```

---

# 📝 Quiz Creation and Grading

The core assessment process works as follows:

```text
Teacher
   │
   ▼
Create Quiz
   │
   ├── Title
   ├── Description
   ├── Subject
   └── Time Limit
   │
   ▼
Add Questions
   │
   ├── Question Text
   ├── Options
   ├── Correct Option
   └── Points
   │
   ▼
Publish Quiz
   │
   ▼
Student Views Quiz
   │
   ▼
Student Selects Answers
   │
   ▼
Submit Quiz
   │
   ▼
Backend Loads Questions
   │
   ▼
Compare Student Answers
   │
   ▼
Calculate Earned Points
   │
   ▼
Calculate Percentage
   │
   ▼
Save Submission
   │
   ▼
Save Result + Grades
```

### Score calculation

The backend calculates:

```text
Percentage =
(Earned Points × 100) / Total Points
```

Each question contributes its configured point value when the student's selected option matches the stored correct option.

---

# 🔐 Authentication & Sessions

QuizMaster uses token-based authentication.

### Login flow

```text
User
 │
 │ email + password + user type
 ▼
POST /api/login
 │
 ▼
Authenticate against SQLite
 │
 ├── Invalid ──► 401 Unauthorized
 │
 └── Valid
       │
       ▼
Generate 32-character session token
       │
       ▼
Store session in SQLite
       │
       ▼
Return token + user information
```

The frontend stores authentication information using browser `localStorage`.

Authenticated API requests send the token through the:

```text
Authorization
```

header.

---

# 🌐 API Reference

The application exposes its backend functionality through `/api` endpoints.

## 🔐 Authentication

### Login

```http
POST /api/login
```

Parameters:

```text
email
password
userType
```

Supported user types include:

```text
admin
teacher
student
```

---

### Registration

```http
POST /api/register
```

Parameters:

```text
name
email
password
userType
extra
```

For students, `extra` is used as the enrollment number.

For teachers, `extra` is used as the employee ID.

---

### Logout

```http
POST /api/logout
```

---

# 📝 Quiz APIs

### Get quizzes

```http
GET /api/quizzes
```

Teachers receive quizzes associated with their account.

Students receive available published quizzes.

---

### Create quiz

```http
POST /api/quizzes
```

Parameters:

```text
title
description
subject
```

Teacher authentication is required.

---

### Get quiz questions

```http
GET /api/quiz-questions
```

Parameter:

```text
quizId
```

Only authenticated students can retrieve questions through this endpoint, and the quiz must be published.

---

### Submit quiz

```http
POST /api/submit-quiz
```

Parameters:

```text
quizId
answers
```

The backend:

1. Loads the quiz questions.
2. Parses submitted answers.
3. Compares answers against correct options.
4. Calculates earned points.
5. Calculates percentage.
6. Saves the submission.
7. Creates the result.
8. Creates question-level grade records.

---

# 👨‍🏫 Teacher APIs

### Get teachers

```http
GET /api/teachers
```

### Get students

```http
GET /api/students
```

### Get teacher results

```http
GET /api/teacher/results
```

### Publish quiz

```http
POST /api/quizzes/publish
```

### Update quiz

```http
POST /api/quizzes/update
```

### Get questions

```http
GET /api/questions
```

### Create/update questions

```http
POST /api/questions
```

---

# 👥 User Management APIs

### Get all users

```http
GET /api/users
```

### Delete user

```http
POST /api/delete-user
```

### Update user role

The backend's user model supports role updates for administrative management.

---

# 💬 Messaging APIs

### Get messages

```http
GET /api/messages
```

### Send message

```http
POST /api/messages/send
```

### Get conversation

```http
GET /api/messages/conversation
```

Parameter:

```text
userId
```

### Broadcast message to students

```http
POST /api/messages/send-broadcast
```

Only authenticated teachers can use the broadcast endpoint.

---

# 🔔 Notification APIs

### Send notification

```http
POST /api/notifications/send
```

### Get notifications

```http
GET /api/notifications
```

### Mark notification as read

```http
POST /api/notifications/mark-read
```

The notification model supports:

- Read/unread state
- Notification types
- User-specific notifications
- Mark-one-as-read
- Mark-all-as-read functionality

---

# 🖥️ Frontend

The project contains five primary frontend pages:

| Page | Purpose |
|---|---|
| `index.html` | Main/landing page |
| `login.html` | Login and registration |
| `admin-dashboard.html` | Administrator interface |
| `teacher-dashboard.html` | Teacher interface |
| `student-dashboard.html` | Student interface |

---

## 🎨 Frontend Design

The teacher dashboard uses a colorful educational UI based around:

- Purple `#8A2BE2`
- Pink `#FF6BCB`
- Soft lavender backgrounds
- White dashboard cards
- Font Awesome icons

The interface includes:

- Sidebar navigation
- Dashboard cards
- Statistics
- Tables
- Modals
- Quiz forms
- Question builders
- Chat interface
- Notification panels

---

# 🧰 Prerequisites

To build the project, you should have:

### Required

- C++17-compatible compiler
- CMake
- Make or a compatible build tool
- SQLite3
- Git
- A modern web browser

The project includes SQLite source/header files:

```text
backend/sqlite3.c
backend/sqlite3.h
```

and the supplied build directory contains a Windows SQLite DLL.

---

# 🚀 Installation

## 1. Clone the repository

```bash
git clone https://github.com/YOUR_USERNAME/quizmaster.git
cd quizmaster
```

---

## 2. Enter the backend directory

```bash
cd "Quiz Application/backend"
```

---

## 3. Create a clean build directory

From the backend directory:

```bash
mkdir build
cd build
```

---

## 4. Configure CMake

```bash
cmake ..
```

---

## 5. Build

```bash
cmake --build .
```

The project defines an executable named:

```text
quiz_server
```

---

# ▶️ Running the Application

The server is configured to run on:

```text
Port: 8080
```

Run the generated server executable.

Then open:

```text
http://localhost:8080/
```

---

# ⚠️ Important Configuration Note

The supplied `main.cpp` contains a machine-specific frontend path similar to:

```cpp
server.staticFiles("E:/BSSE-5B/OOAD/project/Quiz Application/Frontend");
```

This path will not work on another computer unless the project exists at exactly that location.

### Recommended improvement

Replace the hard-coded path with a configurable or dynamically resolved path.

For example:

```text
Quiz Application/
├── backend/
└── Frontend/
```

The server should determine the frontend directory relative to the executable/project rather than depending on a specific Windows drive.

---

# 🔑 Default Administrator Account

The database initialization creates:

```text
Email:    admin@quizmaster.com
Password: admin123
Role:     admin
```

This is implemented directly in the database initialization code.

> ⚠️ **Development use only:** The default password should be changed before any real-world deployment.

---

# 🧪 Example API Request

## Login

Using a query-string based request:

```text
POST /api/login?email=admin%40quizmaster.com&password=admin123&userType=Administrator
```

The server normalizes:

```text
Administrator → admin
```

and validates the credentials.

A successful login returns information similar to:

```json
{
  "success": true,
  "token": "SESSION_TOKEN",
  "user": {
    "id": 1,
    "name": "Administrator",
    "email": "admin@quizmaster.com",
    "userType": "admin"
  }
}
```

---

# 🔄 Typical User Journey

## Teacher

```text
Register/Login
      ↓
Teacher Dashboard
      ↓
Create Quiz
      ↓
Add Questions
      ↓
Add Options
      ↓
Select Correct Answers
      ↓
Assign Points
      ↓
Publish
      ↓
Students Attempt Quiz
      ↓
Review Results
```

## Student

```text
Register/Login
      ↓
Student Dashboard
      ↓
View Published Quizzes
      ↓
Open Quiz
      ↓
Answer Questions
      ↓
Submit
      ↓
Automatic Grading
      ↓
View Result
```

## Administrator

```text
Login
  ↓
Admin Dashboard
  ↓
View Users
  ├── Delete User
  └── Manage Roles
```

---

# 🧱 Object-Oriented Design

The backend is structured around C++ classes.

## User hierarchy

```text
             User
               │
      ┌────────┼─────────┐
      ▼        ▼         ▼
   Student   Teacher    Admin
```

## Quiz domain

```text
Quiz
 │
 └── Question
       │
       └── Options
```

## Result domain

```text
Submission
    │
    ▼
  Result
    │
    └── Grade
          │
          └── Question
```

This structure allows the application to separate domain responsibilities into reusable model classes.

---

# 📦 Backend Components

| Component | Responsibility |
|---|---|
| `Server` | HTTP server, requests, responses and static files |
| `Router` | Routing abstraction |
| `RequestHandler` | Request processing utilities |
| `Database` | SQLite initialization and connection |
| `User` | Base user model |
| `Student` | Student-specific functionality |
| `Teacher` | Teacher-specific functionality |
| `Admin` | Administrative functionality |
| `Quiz` | Quiz management |
| `Question` | Question and answer options |
| `Submission` | Student submissions |
| `Result` | Quiz scores/results |
| `Grade` | Question-level grading |
| `Session` | Authentication sessions |
| `Message` | User messaging |
| `Notification` | User notifications |
| `Json` | JSON-related utility functionality |

---

# 🔒 Security Considerations

The supplied project is an **academic/development application**, not a production-hardened system.

Before deploying publicly, the following should be improved.

## Password hashing

Passwords are currently handled directly in the database.

Production applications should use a secure password hashing algorithm such as:

```text
Argon2
bcrypt
scrypt
PBKDF2
```

---

## Secure session handling

The application uses generated tokens stored in SQLite.

Recommended production improvements:

- Token expiration enforcement
- Secure cookie support
- HTTPS
- Token rotation
- Session revocation
- CSRF protection where applicable

---

## Input validation

The server currently accepts several parameters through query strings.

Production improvements should include:

- Strict input validation
- Maximum input lengths
- Type validation
- Better JSON parsing
- SQL/business-rule validation

---

## Hard-coded paths

Machine-specific paths should be removed.

---

## Default credentials

The default administrator password should never remain unchanged in a production deployment.

---

# ⚠️ Known Limitations

The supplied project contains several areas that should be considered during further development.

### 1. Hard-coded frontend directory

The backend currently references a specific Windows filesystem path.

### 2. Plaintext password storage

Passwords are not securely hashed.

### 3. Custom JSON handling

Some JSON/request parsing is implemented manually. A mature JSON library would improve reliability.

### 4. Query-string API design

Several POST operations receive data through query parameters rather than a conventional JSON request body.

### 5. Generated build files included

The supplied ZIP contains CMake-generated build artifacts.

These should generally not be committed to a clean source repository.

### 6. Some model methods are incomplete

The supplied source contains methods such as `Result::findByQuizId()` that are still marked as TODO or return an empty vector.

These should be completed before relying on those methods for production functionality.

---

# 🧹 Recommended `.gitignore`

For GitHub, consider adding:

```gitignore
# CMake
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
Makefile
install_manifest.txt

# Build output
build/
bin/
Debug/
Release/

# Compiled files
*.o
*.obj
*.exe
*.dll

# Database
*.db
*.sqlite
*.sqlite3

# IDE
.vscode/
.idea/

# OS
.DS_Store
Thumbs.db

# Logs
*.log
```

If the demo database is intentionally required for evaluation, keep a clean sample database separately rather than ignoring every database file.

---

# 🗂️ Recommended GitHub Repository Structure

For a cleaner public repository, I recommend:

```text
quizmaster/
│
├── backend/
│   ├── database/
│   ├── models/
│   ├── server/
│   ├── utils/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── sqlite3.c
│   └── sqlite3.h
│
├── Frontend/
│   ├── index.html
│   ├── login.html
│   ├── admin-dashboard.html
│   ├── teacher-dashboard.html
│   └── student-dashboard.html
│
├── docs/
│   ├── architecture.md
│   ├── database.md
│   └── api.md
│
├── screenshots/
│   ├── login.png
│   ├── student-dashboard.png
│   ├── teacher-dashboard.png
│   └── admin-dashboard.png
│
├── .gitignore
├── README.md
└── LICENSE
```

---

# 📸 Screenshots

Add screenshots to the repository and replace the placeholders below.

### 🔐 Login

```text
screenshots/login.png
```

### 👨‍🎓 Student Dashboard

```text
screenshots/student-dashboard.png
```

### 👨‍🏫 Teacher Dashboard

```text
screenshots/teacher-dashboard.png
```

### 🛡️ Admin Dashboard

```text
screenshots/admin-dashboard.png
```

Once screenshots are added to GitHub, they can be displayed using:

```markdown
![Login](screenshots/login.png)
```

---

# 🛣️ Future Improvements

Potential improvements for the next version include:

- 🔐 Password hashing
- 🔑 Password reset functionality
- ⏱️ Enforced quiz time limits
- 📊 Advanced analytics
- 📈 Teacher performance dashboards
- 🏆 Leaderboards
- 📜 Printable result certificates
- 🔍 Quiz search and filtering
- 📚 More question types
- ✏️ Better rich quiz editor
- 📱 Fully responsive mobile UI
- 🌐 Production-ready HTTP/HTTPS configuration
- 🔒 Secure cookies and session expiration
- 🧪 Unit and integration testing
- 🧰 Automated CMake build pipeline
- 🐳 Docker support
- 📖 API documentation with OpenAPI/Swagger
- 🗃️ Database migration system
- 🧑‍💻 CI/CD with GitHub Actions

---

# 🧪 Testing Recommendations

A future test suite should cover:

### Authentication

- Valid login
- Invalid password
- Invalid user type
- Duplicate registration
- Logout
- Invalid session token

### Quiz Management

- Create quiz
- Update quiz
- Publish quiz
- Unpublish quiz
- Add question
- Update question
- Delete question
- Add options

### Quiz Attempt

- Retrieve published quiz
- Reject unpublished quiz
- Submit answers
- Score correct answers
- Score incorrect answers
- Handle unanswered questions
- Save submission
- Save result

### Communication

- Send message
- Retrieve conversation
- Broadcast message
- Create notification
- Mark notification as read

---

# 🤝 Contributing

Contributions and improvements are welcome.

### 1. Fork the repository

```bash
git fork
```

### 2. Create a feature branch

```bash
git checkout -b feature/your-feature
```

### 3. Make your changes

Follow the existing C++/HTML/JavaScript structure.

### 4. Build and test

```bash
cmake ..
cmake --build .
```

### 5. Commit

```bash
git add .
git commit -m "Add your feature"
```

### 6. Push

```bash
git push origin feature/your-feature
```

### 7. Open a Pull Request

---

# 📄 License

No explicit license file was included in the supplied project.

Before making the repository public, add an appropriate license such as:

- MIT
- Apache 2.0
- GPL-3.0

For an academic submission, a custom project license may also be appropriate.

---

# 👨‍💻 Developer

### Aatazaz Hussain

**QuizMaster — Quiz Application**

This project was developed as an academic/software engineering project demonstrating:

```text
Object-Oriented Programming
+
C++17
+
HTTP Server Development
+
SQLite Database Design
+
Web Frontend Development
+
Authentication
+
Quiz Management
+
Automatic Assessment
```

---

# 📊 Project Summary

| Category | Details |
|---|---|
| 🏷️ Project | QuizMaster |
| 👨‍💻 Developer | Aatazaz Hussain |
| 💻 Backend | C++17 |
| 🌐 Server | Custom C++ HTTP Server |
| 🎨 Frontend | HTML + CSS + JavaScript |
| 🗄️ Database | SQLite |
| 🔐 Authentication | Token-based sessions |
| 👥 Roles | Admin, Teacher, Student |
| 📝 Assessment | Multiple-choice quiz workflow |
| 📊 Grading | Automatic |
| 💬 Communication | Messaging + Broadcast |
| 🔔 Notifications | Supported |
| 🛠️ Build | CMake |
| 📌 Status | Academic / Development Project |

---

# ⚡ Quick Start

```bash
# Clone
git clone https://github.com/YOUR_USERNAME/quizmaster.git

# Enter backend
cd "Quiz Application/backend"

# Configure
mkdir build
cd build
cmake ..

# Build
cmake --build .

# Run the generated quiz server
```

Then visit:

```text
http://localhost:8080/
```

### Development Admin

```text
Email:    admin@quizmaster.com
Password: admin123
```

---

<p align="center">
  <b>🧠 QuizMaster</b><br>
  <i>Create. Challenge. Learn. Measure.</i>
</p>
