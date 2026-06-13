cmake_minimum_required(VERSION 3.28)
project(severance VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(severance src/main.cpp)

enable_testing()
add_executable(severance_tests tests/unit/dummy_test.cpp)
add_test(NAME dummy_test COMMAND severance_tests)
