#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include "../include/common/prefix_parser.h"

namespace po = boost::program_options;

static const std::string PREFIX = "VERSION";

/**
 * Creates a hook file and waits for the response from the filesystem.
 */
void create_command_file(std::string command, const std::string& filepath, const std::string& num = "") {
    auto complete = PrefixParser::apply_prefix(PREFIX, filepath, {std::move(command), num});

    std::ofstream out(complete);

    if (out.is_open()) {
        out << " ";
        out.close();
    } else {
        std::cerr << "Unable to complete command" << std::endl;
        return;
    }

    std::remove(complete.c_str());
}

/**
 * Entry point for the versioning tool.
 *
 * Usage:
 *  ./versioning --help
 *  ./versioning --list --file <file>
 *  ./versioning --restore <version> --file <file>
 *  ./versioning --delete <version> --file <file>
 *  ./versioning --deleteAll --file <file>
 */
int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")                         //
            ("list", "list all versions of a file")                                //
            ("restore", po::value<int>(), "restore a file to a specific version")  //
            ("delete", po::value<int>(), "delete a specific version of a file")    //
            ("deleteAll", "delete all versions of a file")                         //
            ("file", po::value<std::string>(), "file path (is also a positional argument)");

        po::positional_options_description p;
        p.add("file", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 1;
        }

        if (!vm.count("file")) {
            std::cerr << "File path is required" << std::endl;
            return 1;
        }

        std::string file = vm["file"].as<std::string>();

        if (vm.count("list")) {
            create_command_file("list", file);
        }

        if (vm.count("restore")) {
            int version = vm["restore"].as<int>();
            create_command_file("restore", file, std::to_string(version));
        }

        if (vm.count("delete")) {
            int version = vm["delete"].as<int>();
            create_command_file("delete", file, std::to_string(version));
        }

        if (vm.count("deleteAll")) {
            create_command_file("deleteAll", file);
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
