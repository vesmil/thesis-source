cmake_minimum_required(VERSION 3.1.2)

project(fuse++ CXX)

add_library(fuse++ src/fuse_wrapper.cpp)

target_compile_definitions(fuse++ PUBLIC -D_FILE_OFFSET_BITS=64)
target_include_directories(fuse++ PUBLIC include/)

# Link against FUSE libraries
target_link_libraries(fuse++ PRIVATE ${FUSE_LIBRARIES})