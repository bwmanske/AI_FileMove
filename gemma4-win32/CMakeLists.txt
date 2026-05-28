cmake_minimum_required(VERSION 3.14)
project(FileMove VERSION 1.0.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# nlohmann/json
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# GTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(include)

# Main executable
add_executable(FileMove 
    src/main.cpp 
    src/cli_parser.cpp 
    src/storage_manager.cpp
    src/move_worker.cpp
    src/mainwindow.cpp
    src/settings_window.cpp
    src/status_window.cpp
    src/about_window.cpp
    src/group_dialog.cpp
    src/search_dialog.cpp
    src/shutdown_dialog.cpp
    src/logger.cpp
)

target_compile_definitions(FileMove PRIVATE 
    FILEMOVE_VERSION="1.0.4"
    FILEMOVE_BUILD_TIME="2026-05-25 14:30:00"
)

if(WIN32)
    target_link_libraries(FileMove PRIVATE nlohmann_json::nlohmann_json comctl32 gdiplus)
else()
    target_link_libraries(FileMove PRIVATE nlohmann_json::nlohmann_json)
endif()

# Tests
enable_testing()
add_executable(FileMoveTests 
    tests/test_main.cpp 
    src/cli_parser.cpp 
    src/storage_manager.cpp
)
target_link_libraries(FileMoveTests PRIVATE GTest::gtest_main nlohmann_json::nlohmann_json)
include(GoogleTest)
gtest_discover_tests(FileMoveTests)
