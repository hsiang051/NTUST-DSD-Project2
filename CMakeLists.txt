cmake_minimum_required(VERSION 3.10)

project(NTUST_DSD_PROJECT1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# 設定執行檔輸出目錄
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 把 src 資料夾加入
include_directories(${PROJECT_SOURCE_DIR}/include)

# 收集所有 cpp 檔
file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 收集主程序源文件（排除測試文件）
set(MAIN_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/BooleanMinimizer.cpp
)

add_executable(main ${MAIN_SOURCES})

# 生成 test 執行檔
add_executable(test ${PROJECT_SOURCE_DIR}/src/test.cpp)
