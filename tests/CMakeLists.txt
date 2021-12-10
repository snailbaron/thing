link_libraries(thing Catch2::Catch2)

add_executable(basic basic.cpp)
add_test(basic COMMAND basic)
