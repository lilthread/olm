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

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})
