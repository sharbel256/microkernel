cmake_minimum_required(VERSION 3.14)

enable_testing()

find_package(GTest REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/kernel/include)

add_executable(ringbuffer_test test_ringbuffer.cpp)

target_link_libraries(ringbuffer_test PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    kernel
)