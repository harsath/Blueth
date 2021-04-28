cmake_minimum_required(VERSION 3.10)
project(
        test-codec
        LANGUAGES CXX
        DESCRIPTION "CXX Test for codec"
        )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(TEST_CODEC_EXEC_NAME test_codec)
include_directories(../../blueth/codec)
set(
	TEST_CODEC_SOURCE_FILES
	test-line-based-frame-decoder.cpp
        )
add_executable(
	${TEST_CODEC_EXEC_NAME}
	${TEST_CODEC_SOURCE_FILES}
        )
target_link_libraries(
	${TEST_CODEC_EXEC_NAME}
        gtest
        gtest_main
        libblueth
        pthread
        )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
