#pragma once

#include <iostream>

struct LogChannel {
    std::string name;
    LogChannel(const std::string& name) : name(name) {}

    void trace(const std::string& content) {
        std::cout << "TRACE " << channel_prefix(name) << content << std::endl;
    }

    void info(const std::string& content) {
        std::cout << "INFO " << channel_prefix(name) << content << std::endl;
    }

    void warn(const std::string& content) {
        std::cout << "WARN " << channel_prefix(name) << content << std::endl;
    }

    void error(const std::string& content) {
        std::cout << "ERROR " << channel_prefix(name) << content << std::endl;
    }

    std::string prompt(const std::string& str) {
        std::string result;
        std::cout << "PROMPT " << channel_prefix(name) << " >> " << str << ": ";
        std::getline(std::cin, result);
        return result;
    }
private:
    static std::string channel_prefix(const std::string& name) {
        return "[" + name + "] ";
    }
};

LogChannel log(const std::string& chan);