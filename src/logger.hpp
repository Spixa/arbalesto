#pragma once

#include <iostream>

struct LogChannel {
    std::string name;
    LogChannel(const std::string& name) : name(name) {}

    void trace(const std::string& content) {
        std::cout << "T " << channel_prefix(name) << content << std::endl;
    }

    void info(const std::string& content) {
        std::cout << "I " << channel_prefix(name) << content << std::endl;
    }

    void warn(const std::string& content) {
        std::cout << "W " << channel_prefix(name) << content << std::endl;
    }

    void error(const std::string& content) {
        std::cout << "E " << channel_prefix(name) << content << std::endl;
    }
private:
    static std::string channel_prefix(const std::string& name) {
        return "[" + name + "] ";
    }
};

LogChannel log(const std::string& chan);