cmake_minimum_required(VERSION 3.1.2)
project(CustomVFS CXX)
set(CMAKE_CXX_STANDARD 17)

# FUSE
include(FindPkgConfig)
pkg_search_module(FUSE REQUIRED fuse)

# FUSE C++ wrapper
add_subdirectory(libs)

set(CUSTOMVFS_SOURCES src/custom_vfs.cpp include/custom_vfs.h src/file.cpp include/file.h src/directory.cpp include/directory.h include/vfs_node.h include/decorators.h)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# CustomVFS library
add_library(customvfs STATIC ${CUSTOMVFS_SOURCES})
target_include_directories(customvfs PUBLIC include)
target_include_directories(customvfs PRIVATE ${FUSE_INCLUDE_DIRS})
target_link_libraries(customvfs PUBLIC fusexx ${FUSE_LIBRARIES})
set_target_properties(customvfs PROPERTIES LINK_FLAGS "${FUSE_LDFLAGS_OTHER}")

# CustomVFS executable
add_executable(customvfs_exec src/main.cpp)
target_link_libraries(customvfs_exec PRIVATE customvfs)

# Tests
find_package(GTest REQUIRED)

add_executable(customvfs_tests tests/test_main.cpp tests/basic_tests.cpp tests/test_config.h tests/complex_tests.cpp)
target_link_libraries(customvfs_tests PRIVATE GTest::GTest GTest::Main customvfs)
target_include_directories(customvfs_tests PRIVATE ${GTEST_INCLUDE_DIRS})
