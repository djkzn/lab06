cmake_minimum_required(VERSION 3.14)

add_executable(unit_tests TEST.cpp)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/banking
    ${CMAKE_SOURCE_DIR}/third-party/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}/third-party/gtest/googlemock/include
)

target_link_libraries(unit_tests
    banking
    gtest
    gtest_main
    gmock
    gmock_main
)

add_test(NAME unit_tests COMMAND unit_tests)
