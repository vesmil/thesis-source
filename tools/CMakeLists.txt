add_executable(encryption_tool encryption/main.cpp)
target_include_directories(encryption_tool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(encryption_tool PRIVATE Boost::program_options)

add_executable(versioning_tool versioning/main.cpp)
target_include_directories(versioning_tool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(versioning_tool PRIVATE Boost::program_options)