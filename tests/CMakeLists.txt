cmake_minimum_required(VERSION 3.1.2)

project(customvfs_tests CXX)

find_package(GTest REQUIRED)

add_executable(customvfs_tests test_main.cpp basic_vfs_tests.cpp tests_versioning.cpp tests_path.cpp tests_encryption_vfs.cpp tests_prefix.cpp tests_encryptor.cpp)
target_link_libraries(customvfs_tests PRIVATE GTest::GTest GTest::Main customvfs)
