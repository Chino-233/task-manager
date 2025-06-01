cmake_minimum_required(VERSION 3.10)
project(task_manager_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 如果 Crow 的头文件在 extern/crow/include
include_directories(${CMAKE_SOURCE_DIR}/extern/Crow/include)
# MySQL Connector/C++ 的头文件
include_directories(/usr/include/cppconn)

# 有时 link_directories 可省略，CMake 能自动找到 mysqlcppconn
link_directories(/usr/lib/x86_64-linux-gnu)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)    # Crow 可能需要 Boost

# 1. task_manager：主程序
add_executable(task_manager
    src/main.cpp
    src/db_conn.cpp
)

target_link_libraries(task_manager
    PRIVATE
        Threads::Threads
        ${Boost_LIBRARIES}
        mysqlcppconn
)

# 2. db_test：测试程序（如果需要）
add_executable(db_test
    src/db_test.cpp
)
target_link_libraries(db_test
    PRIVATE
        mysqlcppconn
)

add_library(dbconn src/db_conn.cpp)
target_link_libraries(dbconn PRIVATE mysqlcppconn)
target_link_libraries(task_manager PRIVATE dbconn)