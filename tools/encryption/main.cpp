#include <termios.h>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace po = boost::program_options;

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

/**
 * Writes password to file.
 */
void write_password_to_file(const std::string& command_prefix, const std::string& filename,
                            const std::string& password) {
    size_t pos = filename.find_last_of("/\\");
    std::string complete;

    if (pos != std::string::npos) {
        std::string path = filename.substr(0, pos);
        std::string file = filename.substr(pos + 1);
        complete = path + "/" + command_prefix + file;
    } else {
        complete = command_prefix + filename;
    }

    std::ofstream out(complete);
    if (out.is_open()) {
        out << password;
        out.close();
    } else {
        std::cerr << "Unable to complete command" << std::endl;
        return;
    }

    std::ifstream in(complete);
    if (in.is_open()) {
        std::string line;
        std::getline(in, line);
        std::cout << line << std::endl;
        in.close();
    } else {
        std::cerr << "Unable to get response" << std::endl;
        return;
    }

    std::remove(complete.c_str());
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
 *  ./encryption --generate <file>
 */
int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")                                      //
            ("unlock,u", po::value<std::string>(), "unlock a file")                             //
            ("lock,l", po::value<std::string>(), "lock a file")                                 //
            ("key,k", po::value<std::string>(), "custom key to use for encryption/decryption")  //
            ("generate,g", po::value<std::string>(), "generate a key");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (!verify_args(vm)) {
            std::cout << desc << std::endl;
            return 1;
        }

        std::string password;
        if (!vm.count("key")) {
            std::cout << "Enter password (or leave empty for key): ";
            password = get_password();

            if (password.empty()) {
                std::cout << "Default key will be used" << std::endl;
            }
        }

        // TODO handle key and generate
        if (vm.count("unlock")) {
            std::string file = vm["unlock"].as<std::string>();
            write_password_to_file("#unlockPass-", file, password);
        }

        if (vm.count("lock")) {
            std::string file = vm["lock"].as<std::string>();
            write_password_to_file("#lockPass-", file, password);
        }

    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
