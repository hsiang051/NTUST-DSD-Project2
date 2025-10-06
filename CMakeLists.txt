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

# 啟用測試
enable_testing()

add_test(
    NAME test_example
    COMMAND main ${PROJECT_SOURCE_DIR}/res/input.pla ${PROJECT_SOURCE_DIR}/res/output.pla
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

add_test(
    NAME test_input1
    COMMAND main ${PROJECT_SOURCE_DIR}/res/input1.pla ${PROJECT_SOURCE_DIR}/res/output1.pla
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

add_test(
    NAME test_input2
    COMMAND main ${PROJECT_SOURCE_DIR}/res/input2.pla ${PROJECT_SOURCE_DIR}/res/output2.pla
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

add_test(
    NAME test_input3
    COMMAND main ${PROJECT_SOURCE_DIR}/res/input3.pla ${PROJECT_SOURCE_DIR}/res/output3.pla
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

# 設定測試屬性，確保測試總是成功（因為我們主要是想看輸出）
set_tests_properties(test_input1 test_input2 test_input3 PROPERTIES WILL_FAIL FALSE)
