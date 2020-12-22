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
set(EXEC_FILE ./main.cpp)

enable_testing()

add_subdirectory(./vendor/googletest-release-1.10.0)
add_subdirectory(./blueth)
add_subdirectory(./tests)

add_executable(
	${PROJECT_NAME}
	${EXEC_FILE}
	)

target_link_libraries(
	${PROJECT_NAME}
	libblueth
	)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES LINKER_LANGUAGE CXX
	)
