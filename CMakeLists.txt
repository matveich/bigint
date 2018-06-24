cmake_minimum_required(VERSION 3.5.1)
project(bigint)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)

add_executable(big_integer_testing
        big_integer_testing.cpp
        gtest/gtest.h
        gtest/gtest-all.cc
        gtest/gtest_main.cc
        big_integer.h
        big_integer.cpp
        #main.cpp
        my_shared_ptr.h
        my_vector.h
        my_vector.cpp)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -std=c++11 -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -D_GLIBCXX_DEBUG")
endif()

target_link_libraries(big_integer_testing -lpthread)
