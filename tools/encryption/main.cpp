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

int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")         //
            ("unlock", po::value<std::string>(), "unlock a file")  //
            ("lock", po::value<std::string>(), "lock a file");

        // TODO add option for key

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help") ||                            // ask for help
            (!vm.count("unlock") && !vm.count("lock")) ||  // no arguments
            (vm.count("unlock") && vm.count("lock")))      // both arguments
        {
            std::cout << desc << std::endl;
            return 1;
        }

        if (vm.count("unlock")) {
            std::string file = vm["unlock"].as<std::string>();
            std::cout << "Enter password to unlock " << file << ": ";
            std::string password = get_password();
            write_password_to_file("#unlock-", file, password);
        }

        if (vm.count("lock")) {
            std::string file = vm["lock"].as<std::string>();
            std::cout << "Enter password to lock " << file << " (or leave empty for default): ";
            std::string password = get_password();

            if (password.empty()) {
                std::cerr << "Default password is not implemented yet" << std::endl;
                return 0;
            }

            write_password_to_file("#lock-", file, password);
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}
