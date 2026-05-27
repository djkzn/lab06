cmake_minimum_required(VERSION 3.4)
project(banking)

add_library(banking Account.cpp Transaction.cpp)
target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS banking
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
install(FILES Account.h Transaction.h DESTINATION include)
