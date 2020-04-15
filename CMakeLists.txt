cmake_minimum_required(VERSION 3.13.4)

project(chip8)

if (UNIX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -Wpedantic -O2")
endif (UNIX)

if (WIN32)
	set(SFML_STATIC_LIBRARIES TRUE)
endif (WIN32)

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

file(GLOB SRC "*.cpp")
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} sfml-graphics)