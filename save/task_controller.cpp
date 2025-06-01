#include "crow_all.h"
#include "../utils/jwt_util.h"
#include "../db_conn.h"

extern std::shared_ptr<DBConn> db;

void bind_task_routes(crow::SimpleApp &app, const std::string &jwtSecret) {
  // 中间件：验证 JWT
  auto authMiddleware = [jwtSecret](const crow::request &req, crow::response &res, crow::context &ctx) {
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.rfind("Bearer ", 0) != 0) {
      res.code = 401;
      res.write("{\"error\":\"Missing token\"}");
      res.end();
      return;
    }
    std::string token = authHeader.substr(7);
    try {
      auto decoded = JWTUtil::verifyToken(token, jwtSecret);
      ctx["user_id"] = decoded.get_payload_claim("user_id").as_string();
    } catch (...) {
      res.code = 401;
      res.write("{\"error\":\"Invalid token\"}");
      res.end();
    }
  };

  // GET /api/v1/tasks
  CROW_ROUTE(app, "/api/v1/tasks")
    .methods(crow::HTTPMethod::Get)([&](const crow::request &req, crow::response &res, crow::context &ctx) {
      // user_id 已经在 authMiddleware 中放到 ctx 里
      int userId = std::stoi(ctx["user_id"].s());
      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("SELECT id, title, description, priority, status, created_at FROM Task WHERE user_id = ? ORDER BY created_at DESC")
        );
        pstmt->setInt(1, userId);
        std::unique_ptr<sql::ResultSet> result(pstmt->executeQuery());
        crow::json::wvalue resp;
        resp["items"] = crow::json::wvalue::list();
        while (result->next()) {
          crow::json::wvalue item;
          item["id"] = result->getInt("id");
          item["title"] = result->getString("title");
          item["description"] = result->getString("description");
          item["priority"] = result->getInt("priority");
          item["status"] = result->getString("status");
          item["created_at"] = result->getString("created_at");
          resp["items"].push_back(item);
        }
        res.code = 200;
        res.write(resp.dump());
        res.end();
      } catch (...) {
        res.code = 500;
        res.write("{\"error\":\"Failed to fetch tasks\"}");
        res.end();
      }
    }, authMiddleware);

  // POST /api/v1/tasks
  CROW_ROUTE(app, "/api/v1/tasks")
    .methods(crow::HTTPMethod::Post)([&](const crow::request &req, crow::response &res, crow::context &ctx) {
      int userId = std::stoi(ctx["user_id"].s());
      auto x = crow::json::load(req.body);
      if (!x || !x.has("title")) {
        res.code = 400;
        res.write("{\"error\":\"Invalid request\"}");
        res.end();
        return;
      }
      std::string title = x["title"].s();
      std::string desc = x.has("description") ? x["description"].s() : "";
      int priority = x.has("priority") ? x["priority"].i() : 0;
      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("INSERT INTO Task(title, description, priority, user_id) VALUES (?, ?, ?, ?)")
        );
        pstmt->setString(1, title);
        pstmt->setString(2, desc);
        pstmt->setInt(3, priority);
        pstmt->setInt(4, userId);
        pstmt->execute();
        res.code = 201;
        res.write("{\"message\":\"Task created\"}");
        res.end();
      } catch (...) {
        res.code = 500;
        res.write("{\"error\":\"Failed to create task\"}");
        res.end();
      }
    }, authMiddleware);

  // GET /api/v1/tasks/{id}
  CROW_ROUTE(app, "/api/v1/tasks/<int>")
    .methods(crow::HTTPMethod::Get)([&](const crow::request &req, crow::response &res, crow::context &ctx, int id) {
      int userId = std::stoi(ctx["user_id"].s());
      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("SELECT title, description, priority, status FROM Task WHERE id = ? AND user_id = ?")
        );
        pstmt->setInt(1, id);
        pstmt->setInt(2, userId);
        std::unique_ptr<sql::ResultSet> result(pstmt->executeQuery());
        if (result->next()) {
          crow::json::wvalue item;
          item["id"] = id;
          item["title"] = result->getString("title");
          item["description"] = result->getString("description");
          item["priority"] = result->getInt("priority");
          item["status"] = result->getString("status");
          res.code = 200;
          res.write(item.dump());
          res.end();
        } else {
          res.code = 404;
          res.write("{\"error\":\"Task not found\"}");
          res.end();
        }
      } catch (...) {
        res.code = 500;
        res.write("{\"error\":\"Failed to fetch task\"}");
        res.end();
      }
    }, authMiddleware);

  // PUT /api/v1/tasks/{id}
  CROW_ROUTE(app, "/api/v1/tasks/<int>")
    .methods(crow::HTTPMethod::Put)([&](const crow::request &req, crow::response &res, crow::context &ctx, int id) {
      int userId = std::stoi(ctx["user_id"].s());
      auto x = crow::json::load(req.body);
      if (!x || !x.has("title")) {
        res.code = 400;
        res.write("{\"error\":\"Invalid request\"}");
        res.end();
        return;
      }
      std::string title = x["title"].s();
      std::string desc = x.has("description") ? x["description"].s() : "";
      int priority = x.has("priority") ? x["priority"].i() : 0;
      std::string status = x.has("status") ? x["status"].s() : "pending";
      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("UPDATE Task SET title = ?, description = ?, priority = ?, status = ? WHERE id = ? AND user_id = ?")
        );
        pstmt->setString(1, title);
        pstmt->setString(2, desc);
        pstmt->setInt(3, priority);
        pstmt->setString(4, status);
        pstmt->setInt(5, id);
        pstmt->setInt(6, userId);
        int affected = pstmt->executeUpdate();
        if (affected > 0) {
          res.code = 200;
          res.write("{\"message\":\"Task updated\"}");
        } else {
          res.code = 404;
          res.write("{\"error\":\"Task not found or no permission\"}");
        }
        res.end();
      } catch (...) {
        res.code = 500;
        res.write("{\"error\":\"Failed to update task\"}");
        res.end();
      }
    }, authMiddleware);

  // DELETE /api/v1/tasks/{id}
  CROW_ROUTE(app, "/api/v1/tasks/<int>")
    .methods(crow::HTTPMethod::Delete)([&](const crow::request &req, crow::response &res, crow::context &ctx, int id) {
      int userId = std::stoi(ctx["user_id"].s());
      try {
        auto conn = db->getConnection();
        std::unique_ptr<sql::PreparedStatement> pstmt(
          conn->prepareStatement("DELETE FROM Task WHERE id = ? AND user_id = ?")
        );
        pstmt->setInt(1, id);
        pstmt->setInt(2, userId);
        int affected = pstmt->executeUpdate();
        if (affected > 0) {
          res.code = 200;
          res.write("{\"message\":\"Task deleted\"}");
        } else {
          res.code = 404;
          res.write("{\"error\":\"Task not found or no permission\"}");
        }
        res.end();
      } catch (...) {
        res.code = 500;
        res.write("{\"error\":\"Failed to delete task\"}");
        res.end();
      }
    }, authMiddleware);
}
