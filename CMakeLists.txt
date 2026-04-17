cmake_minimum_required(VERSION 3.16)

project(
  cspanish
  VERSION 1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
add_executable(cspanish src/main.cpp)
add_library(headerfiles STATIC)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

target_include_directories(headerfiles PUBLIC src)
target_sources(headerfiles
  PRIVATE
  src/lexer.cpp
  src/nodes.cpp
  src/parser.cpp
  src/sema.cpp
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS src
  FILES
  src/utils.h
  src/tokens.h
  src/lexer.h
  src/nodes.h
  src/parser.h
  src/sema.h
)


target_link_libraries(cspanish
  PRIVATE
    headerfiles
)

# GTEST
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests
  tests/lexer_test.cpp
  tests/parser_test.cpp
  tests/sema_test.cpp
)

target_link_libraries(tests
  PRIVATE
    headerfiles
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
