cmake_minimum_required(VERSION 3.5)

project(view_ibd VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Werror -O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror -O0 -ggdb")


add_subdirectory(deps/libevent)
add_subdirectory(deps/glog)
add_subdirectory(deps/googletest)

#add_subdirectory(ibd_viewer)
add_subdirectory(ibd_parser)
add_subdirectory(table_data_reader)
add_subdirectory(test)
