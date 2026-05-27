cmake_minimum_required(VERSION 3.16)

project(
  olm
  VERSION 1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(olm src/main.cpp)
add_library(headerfiles STATIC)

target_include_directories(headerfiles PUBLIC src)

target_sources(headerfiles
  PRIVATE
    src/utilities.cpp
    src/lexer.cpp
    src/nodes.cpp
    src/parser.cpp
    src/error_manager.cpp
    src/runtime_values.cpp
    src/std.cpp
    src/sema.cpp
    src/interpreter.cpp
    src/repl.cpp
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS src
  FILES
    src/utilities.h
    src/tokens.h
    src/lexer.h
    src/nodes.h
    src/parser.h
    src/error_manager.h
    src/runtime_values.h
    src/std.h
    src/builtins.h
    src/sema.h
    src/interpreter.h
    src/repl.h
)

target_compile_options(headerfiles PRIVATE $<$<CONFIG:Release>:-O3> $<$<CONFIG:Debug>:-g>)
target_compile_options(headerfiles PRIVATE -Wall -Wextra -Werror)

target_compile_options(olm PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
target_link_options(olm PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)


if(NOT WIN32)
  find_library(READLINE_LIB NAMES edit readline)
  target_link_libraries(olm PRIVATE headerfiles ${READLINE_LIB})
else()
  target_link_libraries(olm PRIVATE headerfiles)
endif()


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
  tests/interpreter_test.cpp
)

target_link_libraries(tests
  PRIVATE
    headerfiles
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
