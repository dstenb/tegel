cmake_minimum_required(VERSION 2.6)
project(tegel)

find_package(FLEX)
find_package(BISON)

set (CMAKE_CXX_FLAGS "-std=c++0x -pedantic -Wall -Wextra -Weffc++ -Woverloaded-virtual -Wc++11-compat")
add_subdirectory(src)
