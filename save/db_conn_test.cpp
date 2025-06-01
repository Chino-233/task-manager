#pragma once
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>  // 添加这行
#include <cppconn/resultset.h>  // 添加这行
#include <cppconn/prepared_statement.h>  // 可能也需要
#include <memory>
#include <string>

class DBConn {
public:
    DBConn(const std::string &host, const std::string &user, 
           const std::string &pass, const std::string &db);
    sql::Connection* getConnection();
    
private:
    std::unique_ptr<sql::Connection> conn;
};