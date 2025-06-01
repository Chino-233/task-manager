#include "crow_all.h"
#include "../utils/jwt_util.h"
#include "../db_conn.h"
#include <openssl/sha.h>  // 可用 SHA256 哈希密码，加盐处理最好

extern std::shared_ptr<DBConn> db; // 在 main.cpp 创建并注入

void bind_user_routes(crow::SimpleApp &app, const std::string &jwtSecret) {
  CROW_ROUTE(app, "/api/v1/register").methods(crow::HTTPMethod::Post)(
    [jwtSecret](const crow::request &req) {
      auto x = crow::json::load(req.body);
      if (!x || !x.has("username") || !x.has("password"))
        return crow::response(400, "Invalid request");
      std::string username = x["username"].s();
      std::string password = x["password"].s();
      // 对密码做哈希 (示例用 SHA256, 生产环境推荐 bcrypt / argon2)
      unsigned char hash[SHA256_DIGEST_LENGTH];
      SHA256((unsigned char*)password.c_str(), password.size(), hash);
      std::stringstream ss;
      for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
      std::string hashStr = ss.str();

      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("INSERT INTO User(username, password_hash) VALUES (?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashStr);
        pstmt->execute();
        return crow::response(201, "{\"message\":\"User created\"}");
      } catch (const sql::SQLException &e) {
        return crow::response(500, "{\"error\":\"Registration failed\"}");
      }
    });

  CROW_ROUTE(app, "/api/v1/login").methods(crow::HTTPMethod::Post)(
    [jwtSecret](const crow::request &req) {
      auto x = crow::json::load(req.body);
      if (!x || !x.has("username") || !x.has("password"))
        return crow::response(400, "Invalid request");
      std::string username = x["username"].s();
      std::string password = x["password"].s();
      // 计算哈希
      unsigned char hash[SHA256_DIGEST_LENGTH];
      SHA256((unsigned char*)password.c_str(), password.size(), hash);
      std::stringstream ss;
      for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
      std::string hashStr = ss.str();

      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("SELECT id FROM User WHERE username = ? AND password_hash = ?")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashStr);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
          int userId = res->getInt("id");
          std::string token = JWTUtil::generateToken(std::to_string(userId), jwtSecret);
          crow::json::wvalue resp;
          resp["token"] = token;
          return crow::response{resp};
        } else {
          return crow::response(401, "{\"error\":\"Invalid credentials\"}");
        }
      } catch (const sql::SQLException &e) {
        return crow::response(500, "{\"error\":\"Login failed\"}");
      }
    });
}
