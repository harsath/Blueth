cmake_minimum_required(VERSION 3.10)
project(
	Blueth
	DESCRIPTION "Collection of personal codes used frequently in my projects"
	LANGUAGES CXX
	)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(blueth)
add_subdirectory(tests)
add_subdirectory(vendor/googletest)
