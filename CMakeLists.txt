cmake_minimum_required(VERSION 3.0)

project(CustomVFS)

set(CMAKE_CXX_STANDARD 17)

# Use PkgConfig to find FUSE
include(FindPkgConfig)
pkg_search_module(FUSE REQUIRED fuse)

include_directories(${FUSE_INCLUDE_DIRS})
link_directories(${FUSE_LIBRARY_DIRS})

add_subdirectory(libs)
include_directories(include libs/fusexx/include)

# Source files
set(SOURCES src/CustomVFS.cpp include/CustomVFS.h)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Add library
add_library(customvfs_lib STATIC ${SOURCES})
target_link_libraries(customvfs_lib PRIVATE ${FUSE_LIBRARIES} fusexx)

# Add executable
add_executable(CustomVFS src/main.cpp)
target_link_libraries(CustomVFS PRIVATE customvfs_lib)

# Google Test
find_package(GTest REQUIRED)
add_executable(my_project_tests tests/test_main.cpp tests/basic_tests.cpp)

target_link_libraries(my_project_tests PRIVATE GTest::GTest GTest::Main customvfs_lib)
target_include_directories(my_project_tests PRIVATE ${GTEST_INCLUDE_DIRS})

# Enable testing with CTest
enable_testing()
add_test(NAME my_project_tests COMMAND my_project_tests)