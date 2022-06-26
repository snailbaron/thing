cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(thing INTERFACE)
target_include_directories(thing INTERFACE include)
target_link_libraries(thing)

add_subdirectory(deps)
enable_testing()
add_subdirectory(tests)
