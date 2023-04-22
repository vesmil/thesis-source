#include <boost/program_options.hpp>
#include <filesystem>

#include "config.h"
#include "custom_vfs.h"
#include "encryption_vfs.h"
#include "versioning_vfs.h"

/**
 * Sets up options_description object.
 */
void setup_options(boost::program_options::options_description& desc) {
    desc.add_options()("help,h", "produce this help message")                             //
        ("mountpoint,m", boost::program_options::value<std::string>(), "mountpoint")      //
        ("config,c", boost::program_options::value<std::string>(), "configuration file")  //
        ("test,t", "Create test files inside mount directory.")                           //
        ("fuse-args,f", boost::program_options::value<std::vector<std::string>>(), "FUSE arguments");
}

/**
 * Validates variables_map and prints help message if needed.
 */
bool validate_arguments(const boost::program_options::variables_map& vm,
                        const boost::program_options::options_description& desc) {
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::cout << "mountpoint could be also positional argument" << std::endl;
        return false;
    }

    if (!vm.count("mountpoint")) {
        std::cerr << "Mountpoint not specified" << std::endl;
        return false;
    }

    std::string mountpoint = vm["mountpoint"].as<std::string>();
    if (!std::filesystem::is_directory(mountpoint)) {
        std::cerr << "Mountpoint is not a directory" << std::endl;
        return false;
    }

    return true;
}

/**
 * Parses arguments and stores them in variables_map.
 */
bool parse_arguments(int argc, char* argv[], boost::program_options::variables_map& vm) {
    boost::program_options::options_description desc("Allowed options");

    setup_options(desc);

    boost::program_options::positional_options_description pos_desc;
    pos_desc.add("mountpoint", 1);

    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), vm);
    boost::program_options::notify(vm);

    return validate_arguments(vm, desc);
}

/**
 * Prepares arguments for FUSE main.
 * SImply prepending program name and appending mountpoint.
 */
std::vector<char*> prepare_fuse_arguments(const std::vector<std::string>& fuse_args, const std::string& mountpoint,
                                          const char* argv0) {
    int fuse_argc = static_cast<int>(fuse_args.size()) + 2;
    std::vector<char*> fuse_argv(fuse_argc);

    // Start with program name
    fuse_argv[0] = const_cast<char*>(argv0);

    // Copy arguments
    for (int i = 0; i < fuse_argc - 2; i++) {
        fuse_argv[i + 1] = const_cast<char*>(fuse_args[i].c_str());
    }

    // Add mountpoint
    fuse_argv[fuse_argc - 1] = const_cast<char*>(mountpoint.data());

    return fuse_argv;
}

int main(int argc, char* argv[]) {
    boost::program_options::variables_map vm;
    if (!parse_arguments(argc, argv, vm)) {
        return 1;
    }

    std::string mountpoint = vm["mountpoint"].as<std::string>();
    bool test_mode = vm.count("test") > 0;

    if (vm.count("config")) {
        Config::Parser::ParseFile(vm["config"].as<std::string>());
    }

    CustomVfs custom_vfs(mountpoint, test_mode);
    VersioningVfs versioned(custom_vfs);
    EncryptionVfs encrypted(versioned);

    std::vector<std::string> fuse_args;
    if (vm.count("fuse-args")) {
        fuse_args = vm["fuse-args"].as<std::vector<std::string>>();
    }

    auto fuse_argv = prepare_fuse_arguments(fuse_args, mountpoint, argv[0]);
    encrypted.main(static_cast<int>(fuse_argv.size()), fuse_argv.data());

    return 0;
}
