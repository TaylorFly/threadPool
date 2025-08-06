cmake_minimum_required(VERSION 3.10.0)
project(threadpool VERSION 0.1.0 LANGUAGES C CXX)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(SRC_LIST thread_pool.hpp thread_pool.cpp logger.hpp logger.cpp lockqueue.hpp)

add_executable(threadpool main.cpp ${SRC_LIST})