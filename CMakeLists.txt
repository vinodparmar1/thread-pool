cmake_minimum_required(VERSION 3.16) 

project(threadpool VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#test
add_executable(threadpool-test src/main.cpp)
target_include_directories(threadpool-test PRIVATE include)
target_link_libraries(threadpool-test PRIVATE pthread)

