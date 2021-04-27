cmake_minimum_required(VERSION 3.10)
project(
	test-net
	LANGUAGES CXX
	DESCRIPTION "Test(s) for net"
	)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(../../blueth/http)

add_executable(
	sync_net_stream_client
	./test-SyncNetworkStreamClient.cpp
	)
target_link_libraries(
	sync_net_stream_client
	libblueth
	gtest
	gtest_main
	pthread
	)

add_executable(
	ssl_sync_net_stream_client
	./test-SyncNetworkStreamClientSSL.cpp
	)
target_link_libraries(
	ssl_sync_net_stream_client
	libblueth
	gtest
	gtest_main
	pthread
	)
