cmake_minimum_required(VERSION 3.10)
project(
	test-concurrency
	LANGUAGES CXX
	DESCRIPTION "Test(s) for concurrency"
	)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(../../blueth/http)

add_executable(
	async_event_loop_test
	./test-AsyncEventLoop.cpp
	)
target_link_libraries(
	async_event_loop_test
	libblueth
	)