add_executable(thing-tests thing-tests.cpp)
target_link_libraries(thing-tests PRIVATE thing Catch2::Catch2WithMain)
add_test(NAME thing-tests COMMAND thing-tests)
