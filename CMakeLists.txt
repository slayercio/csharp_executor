cmake_minimum_required(VERSION 4.0)
project(csharp_exec CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES src/**.cpp)

add_library(csharp_exec SHARED ${SOURCES})
target_include_directories(csharp_exec PRIVATE include vendor/json/include)