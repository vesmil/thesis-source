cmake_minimum_required(VERSION 3.1.2)
project(CustomVFS CXX)
set(CMAKE_CXX_STANDARD 17)

# FUSE
include(FindPkgConfig)
pkg_search_module(FUSE REQUIRED fuse)
include_directories(${FUSE_INCLUDE_DIRS})
link_directories(${FUSE_LIBRARY_DIRS})

# Subdirectories, includes, sources
add_subdirectory(libs)
include_directories(include)

set(CUSTOMVFS_SOURCES src/CustomVFS.cpp include/CustomVFS.h)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# CustomVFS library
add_library(customvfs STATIC ${CUSTOMVFS_SOURCES})
target_link_libraries(customvfs PRIVATE fusexx)

# CustomVFS executable
add_executable(customvfs_exec src/main.cpp)
target_link_libraries(customvfs_exec PRIVATE customvfs)

# Tests
find_package(GTest REQUIRED)

add_executable(customvfs_tests tests/test_main.cpp tests/basic_tests.cpp)
target_link_libraries(customvfs_tests PRIVATE GTest::GTest GTest::Main customvfs)
target_include_directories(customvfs_tests PRIVATE ${GTEST_INCLUDE_DIRS})

enable_testing()
add_test(NAME customvfs_tests COMMAND customvfs_tests)
