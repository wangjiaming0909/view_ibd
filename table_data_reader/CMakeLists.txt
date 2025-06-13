cmake_minimum_required(VERSION 3.5)

add_library(table_data_reader SHARED ibdata1_reader.h table_reader.h table_reader.cc)

target_include_directories(table_data_reader PUBLIC ../ibd_parser)
target_link_libraries(table_data_reader PUBLIC ibd_parser)
