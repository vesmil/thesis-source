add_executable(cvfs_encrypt encryption_tool.cpp)
target_include_directories(cvfs_encrypt PRIVATE ${Boost_INCLUDE_DIRS} ../include)
target_link_libraries(cvfs_encrypt PRIVATE Boost::program_options customvfs)

add_executable(cvfs_version versioning_tool.cpp)
target_include_directories(cvfs_version PRIVATE ${Boost_INCLUDE_DIRS} ../include)
target_link_libraries(cvfs_version PRIVATE Boost::program_options customvfs)