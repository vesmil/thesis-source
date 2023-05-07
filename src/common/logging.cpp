#include "common/logging.h"

void Logging::set_logging_file(const std::string& path) {
    file = path;
}

std::string Logging::file;