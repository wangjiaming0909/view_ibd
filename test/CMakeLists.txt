cmake_minimum_required(VERSION 3.5)

add_executable(view_ibd_test test.cc ibd_parser_test.cc)
target_link_libraries(view_ibd_test gtest gmock ibd_parser glog)
include_directories(../ibd_parser)
