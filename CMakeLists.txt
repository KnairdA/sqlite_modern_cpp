cmake_minimum_required(VERSION 2.8)
project(sqlite_modern_cpp_example)

set(
	CMAKE_CXX_FLAGS
	"-std=c++14 -W -Wall -Wextra -pedantic"
)

include_directories(
	src/
)

add_executable(
	example
	example.cc
)

target_link_libraries(
	example
	sqlite3
)
