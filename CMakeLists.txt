cmake_minimum_required(VERSION 3.1.2)
project(CustomVFS CXX)
set(CMAKE_CXX_STANDARD 17)

# Libs
include(FindPkgConfig)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(fuse REQUIRED)

find_package(Sodium REQUIRED)
if (NOT sodium_FOUND)
    pkg_search_module(LIBSODIUM REQUIRED libsodium)
endif ()

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(/usr/local/include)

# FUSE C++ wrapper
add_subdirectory(libs)

# Sources
set(CUSTOMVFS_SOURCES src/custom_vfs.cpp src/encryption_vfs.cpp src/versioning_vfs.cpp src/common/config.cpp src/encryptor.cpp src/common/path.cpp src/common/prefix_parser.cpp include/common/prefix_parser.h src/encryptor_mac.cpp)

# CustomVFS library
add_library(customvfs STATIC ${CUSTOMVFS_SOURCES})
target_include_directories(customvfs PUBLIC include)
target_include_directories(customvfs PRIVATE ${FUSE_INCLUDE_DIRS} ${LIBSODIUM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_link_libraries(customvfs PUBLIC fusexx ${FUSE_LIBRARIES} ${LIBSODIUM_LIBRARIES} ${Boost_LIBRARIES})
set_target_properties(customvfs PROPERTIES LINK_FLAGS "${FUSE_LDFLAGS_OTHER}")

# CustomVFS executable
add_executable(customvfs_exec src/main.cpp)
target_link_libraries(customvfs_exec PRIVATE customvfs)

# Tools
add_subdirectory(tools)

# Tests
add_subdirectory(tests)