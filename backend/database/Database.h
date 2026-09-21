#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

class Database {
private:
    static sqlite3* db;
    static bool isOpen;
    
    static bool executeSQL(const std::string& sql);
    static bool createTables();

public:
    static bool open();
    static void close();
    static sqlite3* getConnection();
    
    static bool userExists(const std::string& email);
    static bool createAdminUser();
    static void initialize();
    
    static std::string getError();
};

#endif