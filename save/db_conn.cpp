#include "db_conn.h"
#include <cppconn/exception.h>
#include <iostream>

DBConn::DBConn(const std::string &host, const std::string &user,
               const std::string &pass, const std::string &db) {
    try {
        driver_ = get_driver_instance();
        conn_.reset(driver_->connect(host, user, pass));
        conn_->setSchema(db);
    } catch (const sql::SQLException &e) {
        std::cerr << "Database connection failed: " << e.what() << std::endl;
        throw;
    }
}

DBConn::~DBConn() {
    // shared_ptr 会自动释放
}

std::shared_ptr<sql::Connection> DBConn::getConnection() {
    return conn_;
}
