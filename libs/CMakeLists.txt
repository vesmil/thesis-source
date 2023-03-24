cmake_minimum_required(VERSION 3.1.2)

project(fusexx CXX)

add_library(fusexx fusexx/src/fuse_wrapper.cpp)

target_compile_definitions(fusexx PUBLIC -D_FILE_OFFSET_BITS=64)
target_include_directories(fusexx PUBLIC fusexx/include)

# Link against FUSE libraries
target_link_libraries(fusexx PRIVATE ${FUSE_LIBRARIES})