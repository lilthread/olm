cmake_minimum_required(VERSION 3.10)

project(
  olm 
  VERSION 1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

include_directories(include)
set(SRC_FILES
    src/lexer.cc
)

# ---------------- LIBRARY -------------------
add_library(olm_lib STATIC ${SRC_FILES})
target_include_directories(olm_lib PUBLIC include)

# ---------------- EXECUTABLE -------------------
add_executable(${CMAKE_PROJECT_NAME} src/main.cc)
target_link_libraries(${PROJECT_NAME}
  PRIVATE olm_lib
)

# ---------------- TESTS -------------------
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SRC tests/lexer_test.cc)

add_executable(tests ${TEST_SRC})

target_link_libraries(tests 
PRIVATE
  olm_lib
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(tests)

