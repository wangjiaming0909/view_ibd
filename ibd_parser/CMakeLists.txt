cmake_minimum_required(VERSION 3.5)

add_library(ibd_parser SHARED
    page.cc page.h
    headers.cc headers.h
    defines.h
    file_space.h
file_space_reader.h file_space_reader.cc)
#aux_source_directory(ibd_parser IBD_PARSER_DIR)
#target_sources(ibd_parser PUBLIC page.cc)
target_link_libraries(ibd_parser glog)
