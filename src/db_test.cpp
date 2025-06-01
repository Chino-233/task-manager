#include <iostream>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

int main()
{
    try {
        sql::Driver *driver = get_driver_instance();
        // 连接到 db 容器，如果是在本机环境，127.0.0.1:3306，用户 tmuser，密码 tmpass
        std::unique_ptr<sql::Connection> conn(
            driver->connect("tcp://127.0.0.1:3306", "tmuser", "tmpass")
        );
        conn->setSchema("task_manager");

        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS test_tbl2 (id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(50));");
        stmt->execute("INSERT INTO test_tbl2(name) VALUES ('world');");

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM test_tbl2;"));
        while (res->next()) {
            std::cout << "id=" << res->getInt("id") << ", name=" << res->getString("name") << std::endl;
        }
    } catch (sql::SQLException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
