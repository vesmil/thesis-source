cmake_minimum_required(VERSION 3.0)

project(CustomVFS)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Source files
set(SOURCES src/main.cpp src/fuse_wrapper.cpp include/fuse_wrapper.h)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Add executable
add_executable(CustomVFS ${SOURCES})

# LibFUSE
find_package(FUSE REQUIRED)
include_directories(${FUSE_INCLUDE_DIR})
target_link_libraries(CustomVFS PRIVATE ${FUSE_LIBRARIES})

# Google Test
find_package(GTest REQUIRED)
add_executable(my_project_tests tests/test_main.cpp tests/basic_tests.cpp)

target_link_libraries(my_project_tests PRIVATE ${GTEST_BOTH_LIBRARIES})
target_include_directories(my_project_tests PRIVATE ${GTEST_INCLUDE_DIRS})
