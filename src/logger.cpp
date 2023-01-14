#include "logger.hpp"

LogChannel log(const std::string& chan) {
    return LogChannel(chan);
}