cmake_minimum_required(VERSION 3.10)
project(
	test-container
	LANGUAGES CXX
	DESCRIPTION "CXX Test for containers"
	)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(TEST_CONTAINER_EXEC_NAME test_container)
set(
	TEST_CONTAINER_SOURCE_FILES
	./test-matrix.cpp
	)
add_executable(
	${TEST_CONTAINER_EXEC_NAME}
	${TEST_CONTAINER_SOURCE_FILES}
	)
target_link_libraries(
	${TEST_CONTAINER_EXEC_NAME}
	gtest_main
	libblueth
	pthread
	)
add_test(
	NAME ${TEST_CONTAINER_EXEC_NAME}
	COMMAND ${TEST_CONTAINER_EXEC_NAME}
	)
