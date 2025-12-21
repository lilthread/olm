cmake_minimum_required(VERSION 3.16)

project(
  olm
  VERSION 1.0
  LANGUAGES C CXX
)

add_library(olm_lib STATIC)

target_sources(olm_lib
  PRIVATE
    src/lexer.cpp
    src/memoryarena.cpp
    src/ast.cpp
    src/parser.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src
    FILES
      src/lexer.h
      src/memoryarena.h
      src/ast.h
      src/parser.h
)

target_include_directories(olm_lib
PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_features(olm_lib
PUBLIC
  cxx_std_23
)

if (MSVC)
  target_compile_options(olm_lib PRIVATE /W4)
else()
  target_compile_options(olm_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()


add_executable(olm src/main.cpp)

target_link_libraries(olm
  PRIVATE
    olm_lib
)

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
)

target_link_libraries(tests
  PRIVATE
    olm_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
