cmake_minimum_required(VERSION 3.5)

add_library(ibd_parser SHARED
    page.cc page.h
    headers.cc headers.h
    ut.h
    file_space.h
file_space_reader.h file_space_reader.cc
dd.h
data.h
dd_table.h
dd_properties.h dd_properties.cc
dd_index.h
buffer_pool.h buffer_pool.cc
dd_cache.h dd_cache.cc
table.h table.cc
parser.h parser.cc
table_loader.h table_loader.cc)


#aux_source_directory(ibd_parser IBD_PARSER_DIR)
#target_sources(ibd_parser PUBLIC page.cc)
target_link_libraries(ibd_parser glog boost_system)

option(TESTING "testing" OFF)
if (${TESTING})
  add_definitions(-DTESTING)
endif()
