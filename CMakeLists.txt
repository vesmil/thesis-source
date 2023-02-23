cmake_minimum_required(VERSION 3.0)

project(CustomVFS)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Source files
set(SOURCES
        src/main.cpp
        tests/test_main.cpp)

# Add executable
add_executable(CustomVFS ${SOURCES})

# Link with libFUSE
find_package(FUSE REQUIRED)
target_link_libraries(my_project PRIVATE ${FUSE_LIBRARIES})
target_include_directories(my_project PRIVATE ${FUSE_INCLUDE_DIRS})

# Link with Google Test
find_package(GTest REQUIRED)
add_executable(my_project_tests
        tests/test_main.cpp
        tests/basic_tests.cpp
        )

target_link_libraries(my_project_tests PRIVATE ${GTEST_BOTH_LIBRARIES})
target_include_directories(my_project_tests PRIVATE ${GTEST_INCLUDE_DIRS})
