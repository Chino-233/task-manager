#pragma once
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <memory>
#include <string>

class DBConn {
public:
    DBConn(const std::string &host, const std::string &user, const std::string &pass, const std::string &db);
    ~DBConn();

    // 返回原始 Connection 指针，供业务层使用
    std::shared_ptr<sql::Connection> getConnection();

private:
    sql::Driver* driver_;
    std::shared_ptr<sql::Connection> conn_;
};
