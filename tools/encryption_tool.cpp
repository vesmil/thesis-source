#include <termios.h>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include "common/path.h"
#include "common/prefix_parser.h"
#include "hook-generation/encryption.h"

namespace po = boost::program_options;

/// @brief Sets terminal echo - used to hide password input.
void set_stdin_echo(bool enable) {
    struct termios tty {};
    tcgetattr(STDIN_FILENO, &tty);
    if (!enable)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/// @brief Reads password from stdin.
std::string get_password() {
    std::string password;
    set_stdin_echo(false);
    std::getline(std::cin, password);
    set_stdin_echo(true);
    return password;
}

/// @brief Writes to a command file
bool perform_command(const std::string& command) {
    std::ofstream out(command);

    if (out.is_open()) {
        out << " ";
        out.close();
    } else {
        std::cerr << "Unable to perform command" << std::endl;
        return false;
    }

    std::remove(command.c_str());

    return true;
}

/// @brief Writes password to a command file.
void password_command(const std::string& command, const std::string& password) {
    std::ofstream out(command);
    if (out.is_open()) {
        out << password;
        out.close();
    } else {
        std::cerr << "Unable to perform command" << std::endl;
    }

    std::remove(command.c_str());
}

/// @brief Verifies that the arguments are valid.
bool verify_args(const po::variables_map& vm) {
    if (!vm.count("unlock") && !vm.count("lock") && !vm.count("generate")) {
        std::cout << "No action specified" << std::endl;
        return false;
    }

    if (vm.count("generate") && vm.count("key")) {
        std::cout << "Cannot generate key and use custom key at the same time" << std::endl;
        return false;
    }

    if (vm.count("default-lock") && !vm.count("locks")) {
        std::cout << "Cannot use a default lock without specifying locks" << std::endl;
        return false;
    }

    if (vm.count("default-lock") && vm.count("unlock")) {
        std::cout << "Cannot use a default lock and unlock at the same time" << std::endl;
        return false;
    }

    if (vm.count("default-lock") && vm.count("key")) {
        std::cout << "Cannot use a default key and a custom key at the same time" << std::endl;
        return false;
    }

    if (vm.count("unlock") && vm.count("lock")) {
        std::cout << "Cannot lock and unlock at the same time" << std::endl;
        return false;
    }

    if (vm.count("unlock") && vm.count("generate")) {
        std::cout << "Cannot unlock and generate key at the same time" << std::endl;
        return false;
    }

    if (vm.count("set-key-path") && vm.count("generate") || vm.count("set-key-path") && vm.count("key")) {
        std::cout << "Cannot set key path and generate key or use custom key at the same time" << std::endl;
    }

    return true;
}

/**
 * @brief Entry point for the encryption tool.
 *
 * @details Usage:                           \n
 *  ./encryption --help                      \n
 *  ./encryption --unlock <file>             \n
 *  ./encryption --lock <file>               \n
 *  ./encryption --lock <file> --key <key>   \n
 *  ./encryption --unlock <file> --key <key> \n
 *                                           \n
 *  ./encryption --generate <file>           \n
 *  ./encryption --set-key-path <file>
 */
int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")                                      //
            ("unlock,u", po::value<std::string>(), "unlock a file")                             //
            ("lock,l", po::value<std::string>(), "lock a file")                                 //
            ("default-lock,d", "lock a file with default key")                                  //
            ("key,k", po::value<std::string>(), "custom key to use for encryption/decryption")  //
            ("set-key-path,s", po::value<std::vector<std::string>>(),
             "requires two args - <vfs> and <key-path> - it sets a default path for key")  //
            ("generate,g", po::value<std::string>(), "generate a key into chosen file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (!verify_args(vm)) {
            std::cout << desc << std::endl;
            return 1;
        }

        std::string password;

        std::string key;
        bool use_key = false;

        if (vm.count("key")) {
            key = vm["key"].as<std::string>();
            key = Path::to_absolute(key);

            use_key = true;
        }

        if (vm.count("generate")) {
            key = vm["generate"].as<std::string>();
            key = Path::to_absolute(key);

            use_key = true;
        }

        // If no key is provided, ask for password.
        if (!use_key) {
            std::cout << "Enter password (or leave empty for key): ";
            password = get_password();
        } else if (vm.count("generate")) {
            std::string file = vm["generate"].as<std::string>();

            if (!perform_command(EncryptionHookGenerator::generate_key_hook(file))) {
                return 2;
            }

            key = Path::to_absolute(file);
        }

        // Does the main operation.
        if (vm.count("set-key-path")) {
            std::vector<std::string> paths = vm["set-key-path"].as<std::vector<std::string>>();
            if (paths.size() != 2) {
                std::cerr << "Invalid number of arguments for set-key-path" << std::endl;
                std::cerr << "Usage: ./encryption --set-key-path <path-to-vfs> <path-to-key-file>" << std::endl;
                return 1;
            }

            perform_command(
                EncryptionHookGenerator::set_key_path_hook(Path::to_absolute(paths[0]), Path::to_absolute(paths[0])));

        } else if (vm.count("unlock")) {
            std::string file = vm["unlock"].as<std::string>();
            if (use_key) {
                perform_command(EncryptionHookGenerator::unlock_key_hook(Path::to_absolute(file), key));
            } else {
                password_command(EncryptionHookGenerator::unlock_pass_hook(Path::to_absolute(file)), password);
            }
        } else if (vm.count("lock")) {
            std::string file = vm["lock"].as<std::string>();
            if (vm.count("key")) {
                perform_command(EncryptionHookGenerator::lock_key_hook(Path::to_absolute(file), key));
            } else {
                password_command(EncryptionHookGenerator::lock_pass_hook(Path::to_absolute(file)), password);
            }
        } else if (vm.count("default-lock")) {
            std::string file = vm["default-lock"].as<std::string>();
            perform_command(EncryptionHookGenerator::default_lock_hook(Path::to_absolute(file)));
        }

    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
