cmake_minimum_required(VERSION 3.10)
project(QuizServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include everything inside backend
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add executable with all .cpp files
add_executable(quiz_server
    main.cpp

    sqlite3.c

    database/Database.cpp

    models/User.cpp
    models/Quiz.cpp
    models/Result.cpp
    models/Message.cpp
    models/Session.cpp

    server/Server.cpp
    server/Router.cpp
    server/RequestHandler.cpp

    utils/Json.cpp
)

# Link WinSock on Windows
if(WIN32)
    target_link_libraries(quiz_server ws2_32)
endif()
