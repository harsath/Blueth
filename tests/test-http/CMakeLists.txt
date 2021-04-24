cmake_minimum_required(VERSION 3.10)
project(
	test-http
	LANGUAGES CXX
	DESCRIPTION "CXX Test for http"
	)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(TEST_HTTP_EXEC_NAME test_http)
include_directories(../../blueth/http)
set(
	TEST_HTTP_SOURCE_FILES
	./test-http-message.cpp
	./test-http-state-machine.cpp
	./test-http-response-message.cpp
	)
add_executable(
	${TEST_HTTP_EXEC_NAME}
	${TEST_HTTP_SOURCE_FILES}
	)
target_link_libraries(
	${TEST_HTTP_EXEC_NAME}
	gtest
	gtest_main
	libblueth
	pthread
	)
add_test(
	NAME ${TEST_HTTP_EXEC_NAME}
	COMMAND ${TEST_HTTP_EXEC_NAME}
	)
