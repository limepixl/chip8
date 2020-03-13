cmake_minimum_required(VERSION 3.12)
project(chip8)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O2 -Wall -Wextra")

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

file(GLOB SRC "*.cpp")
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-audio)
