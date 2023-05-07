cmake_minimum_required(VERSION 3.1.2)

project(customvfs_tests CXX)

find_package(GTest REQUIRED)

add_executable(customvfs_tests test_main.cpp basic_vfs_tests.cpp versioning_vfs_tests.cpp path_tests.cpp encryption_vfs_tests.cpp)
target_link_libraries(customvfs_tests PRIVATE GTest::GTest GTest::Main customvfs)
