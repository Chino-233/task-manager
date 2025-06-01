#include "crow_all.h"
#include "db_conn.h"

#include <memory>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

int main()
{
    crow::SimpleApp app;

    auto db = std::make_shared<DBConn>(
        "tcp://127.0.0.1:3306", "tmuser", "tmpass", "task_manager"
    );

    // 健康检查路由
    CROW_ROUTE(app, "/health")([](){
        return crow::json::wvalue({{"status", "ok"}});
    });

    // dbtest 路由：读取 ?name=XXX，如果没有就 default 为 "anonymous"
    CROW_ROUTE(app, "/dbtest")
    ([db](const crow::request &req) {
        // 先从 req.url_params.get 里去找 "name"
        const char* q = req.url_params.get("name");
        std::string name = (q ? std::string(q) : std::string("anonymous"));

        try {
            auto conn = db->getConnection();
            std::unique_ptr<sql::Statement> stmt(conn->createStatement());

            // 确保表存在
            stmt->execute(
                "CREATE TABLE IF NOT EXISTS api_test ("
                "id INT PRIMARY KEY AUTO_INCREMENT, "
                "name VARCHAR(50)"
                ");"
            );

            // 插入一条新的记录
            std::string insert_sql = "INSERT INTO api_test(name) VALUES ('" + name + "');";
            stmt->execute(insert_sql);

            // 查询最新五条
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(
                "SELECT * FROM api_test ORDER BY id DESC LIMIT 5;"
            ));

            // 构造 JSON
            crow::json::wvalue result;
            result["items"] = crow::json::wvalue::list();
            auto &items = result["items"];

            int idx = 0;
            while (res->next()) {
                crow::json::wvalue row;
                row["id"]   = res->getInt("id");
                row["name"] = res->getString("name");
                items[idx++] = std::move(row);
            }
            return crow::response{result};
        }
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["error"] = e.what();
            return crow::response{500, err};
        }
    });

    app.port(8080).multithreaded().run();
    return 0;
}
