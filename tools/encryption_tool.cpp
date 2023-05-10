#include <termios.h>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "common/prefix_parser.h"
#include "hook-generation/encryption.h"

namespace po = boost::program_options;
std::string const PREFIX = Config::encryption.prefix;

/**
 * Sets terminal echo - used to hide password input.
 */
void set_stdin_echo(bool enable) {
    struct termios tty {};
    tcgetattr(STDIN_FILENO, &tty);
    if (!enable)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/**
 * Reads password from stdin.
 */
std::string get_password() {
    std::string password;
    set_stdin_echo(false);
    std::getline(std::cin, password);
    set_stdin_echo(true);
    return password;
}

bool perform_command(std::string command, const std::string& filename) {
    std::string prefixed_file = PrefixParser::apply_prefix(PREFIX, filename, {std::move(command)});

    std::ofstream out(prefixed_file);
    if (out.is_open()) {
        out << " ";
        out.close();
    } else {
        std::cerr << "Unable to perform command" << std::endl;
        return false;
    }

    return true;
}

bool perform_command_arg(const std::string& command, const std::string& filename, const std::string& arg) {
    std::string prefixed_file = PrefixParser::apply_prefix(PREFIX, filename, {command, arg});

    std::ofstream out(prefixed_file);
    if (out.is_open()) {
        out << " ";
        out.close();
    } else {
        std::cerr << "Unable to perform command" << std::endl;
        return false;
    }

    return true;
}

/**
 * Writes password to file.
 */
void password_command(const std::string& command_prefix, const std::string& filename, const std::string& password) {
    std::string prefixed_file = PrefixParser::apply_prefix(PREFIX, filename, {command_prefix});

    std::ofstream out(prefixed_file);
    if (out.is_open()) {
        out << password;
        out.close();
    } else {
        std::cerr << "Unable to unlock file" << std::endl;
    }
}

bool verify_args(const po::variables_map& vm) {
    if (!vm.count("unlock") && !vm.count("lock") && !vm.count("generate")) {
        std::cout << "No action specified" << std::endl;
        return false;
    }

    if (vm.count("generate") && vm.count("key")) {
        std::cout << "Cannot generate key and use custom key at the same time" << std::endl;
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
 * Entry point for the encryption tool.
 *
 * Usage:
 *  ./encryption --help
 *  ./encryption --unlock <file>
 *  ./encryption --lock <file>
 *  ./encryption --lock <file> --key <key>
 *  ./encryption --unlock <file> --key <key>
 *
 *  ./encryption --generate <file>
 *  ./encryption --set-key-path <file>
 */
int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")                                      //
            ("unlock,u", po::value<std::string>(), "unlock a file")                             //
            ("lock,l", po::value<std::string>(), "lock a file")                                 //
            ("key,k", po::value<std::string>(), "custom key to use for encryption/decryption")  //

            ("set-key-path,s", po::value<std::string>(), "sets a default path for key")  //
            ("generate,g", po::value<std::string>(), "generate a key into chosen file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (!verify_args(vm)) {
            std::cout << desc << std::endl;
            return 1;
        }

        std::string password;

        std::string key = vm["key"].as<std::string>();
        bool use_key = vm.count("key");

        if (!use_key) {
            std::cout << "Enter password (or leave empty for key): ";
            password = get_password();
        } else if (vm.count("generate")) {
            std::string file = vm["generate"].as<std::string>();
            if (!perform_command("generate", file)) {
                return 2;
            }

            key = file;
        }

        if (vm.count("set-key-path")) {
            std::string file = vm["set-key-path"].as<std::string>();
            perform_command("setKeyPath", file);
        } else if (vm.count("unlock")) {
            std::string file = vm["unlock"].as<std::string>();
            if (use_key) {
                perform_command_arg("unlock", file, key);
            } else {
                password_command("unlock", file, password);
            }
        } else if (vm.count("lock")) {
            std::string file = vm["lock"].as<std::string>();
            if (vm.count("key")) {
                perform_command_arg("lock", file, key);
            } else {
                password_command("lock", file, password);
            }
        }

    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
