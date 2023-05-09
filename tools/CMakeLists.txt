add_executable(encryption_tool encryption_tool.cpp ../src/common/prefix_parser.cpp)

target_include_directories(encryption_tool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(encryption_tool PRIVATE Boost::program_options)

add_executable(versioning_tool versioning_tool.cpp ../src/common/prefix_parser.cpp)
target_include_directories(versioning_tool PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(versioning_tool PRIVATE Boost::program_options)