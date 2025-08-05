cmake_minimum_required(VERSION 3.10.0)
project(threadpool VERSION 0.1.0 LANGUAGES C CXX)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(SRC_LIST threadPool.cpp threadPool.hpp any.hpp Result.hpp Result.cpp semaphore.hpp semaphore.cpp)

add_executable(threadpool main.cpp ${SRC_LIST})

add_library(pool STATIC  ${SRC_LIST})
