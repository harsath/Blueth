cmake_minimum_required(VERSION 3.10)
project(
        test-io
        LANGUAGES CXX
        DESCRIPTION "CXX Test for io"
        )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(TEST_IO_EXEC_NAME test_io)
include_directories(../../blueth/io)
set(
	TEST_IO_SOURCE_FILES
	./test-IOBuffer.cpp
        )
add_executable(
	${TEST_IO_EXEC_NAME}
        ${TEST_IO_SOURCE_FILES}
        )
target_link_libraries(
	${TEST_IO_EXEC_NAME}
        gtest
        gtest_main
        libblueth
        pthread
        )
add_test(
	NAME ${TEST_IO_EXEC_NAME}
        COMMAND ${TEST_IO_EXEC_NAME}
        )
