cmake_minimum_required(VERSION 3.5)

add_library(ibd_parser SHARED
    page.cc page.h
    headers.cc headers.h
    defines.h
    file_space.h
file_space_reader.h file_space_reader.cc
dd.h
dd_table.h
dd_properties.h
dd_index.h
design.h
dd_cache.h
table.h
parser.h parser.cc)


#aux_source_directory(ibd_parser IBD_PARSER_DIR)
#target_sources(ibd_parser PUBLIC page.cc)
target_link_libraries(ibd_parser glog)

option(TESTING "testing" OFF)
if (${TESTING})
  add_definitions(-DTESTING)
endif()
