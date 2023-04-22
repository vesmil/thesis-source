#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace po = boost::program_options;

void create_command_file(const std::string& command, const std::string& file, const std::string& num = "") {
    // TODO solve the problem with relative path

    std::string filename = "#" + command + "-" + file;
    if (!num.empty()) {
        filename += "_" + num;
    }

    std::ofstream out(filename);
    if (out.is_open()) {
        out.close();
    } else {
        std::cerr << "Unable to create file: " << filename << std::endl;
    }
}

int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")                       //
            ("list", "list all versions of a file")                              //
            ("restore", "restore a file to a specific version")                  //
            ("delete", po::value<int>(), "delete a specific version of a file")  //
            ("delete_all", "delete all versions of a file")                      //
            ("file", po::value<std::string>(), "file path");

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
            create_command_file("restore", file);
        }

        if (vm.count("delete")) {
            int version = vm["delete"].as<int>();
            create_command_file("delete", file, std::to_string(version));
        }

        if (vm.count("delete_all")) {
            create_command_file("delete_all", file);
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
