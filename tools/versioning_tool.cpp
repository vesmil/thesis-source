#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include "hook-generation/versioning.h"

namespace po = boost::program_options;

/// @brief Writes to a command file
bool perform_command(const std::string& command, bool read = false) {
    std::ofstream out(command);

    if (out.is_open()) {
        out << " ";
        out.close();
    } else {
        std::cerr << "Unable to perform command" << std::endl;
        return false;
    }

    if (read) {
        std::ifstream in(command);
        std::string response;
        std::getline(in, response);
        std::cout << response << std::endl;

        in.close();
    }

    std::remove(command.c_str());

    return true;
}

/**
 * @brief Entry point for the versioning tool.
 *
 * @details Usage:                                    \n
 *  ./versioning --help                               \n
 *  ./versioning --list --file <file>                 \n
 *  ./versioning --restore <version> --file <file>    \n
 *  ./versioning --delete <version> --file <file>     \n
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
        file = Path::to_absolute(file);

        if (vm.count("list")) {
            perform_command(VersioningHookGenerator::list_hook(file), true);
        }

        if (vm.count("restore")) {
            int version = vm["restore"].as<int>();
            perform_command(VersioningHookGenerator::restore_hook(file, std::to_string(version)));
        }

        if (vm.count("delete")) {
            int version = vm["delete"].as<int>();
            perform_command(VersioningHookGenerator::delete_hook(file, std::to_string(version)));
        }

        if (vm.count("deleteAll")) {
            perform_command(VersioningHookGenerator::delete_all_hook(file));
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
