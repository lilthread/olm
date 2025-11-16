cmake_minimum_required(VERSION 3.10)

project(
  olm 
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


file(GLOB SRC_FILES src/*.cc)
include_directories(include)


add_library(olm_lib STATIC ${SRC_FILES}) 
add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

### test
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  tests tests/lexer_test.cc
)
target_link_libraries(
  tests 
  olm_lib
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(tests)

