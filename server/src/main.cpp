#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "server.h"

void setup_logger() {
    auto console = spdlog::stdout_color_mt("server");

    spdlog::set_default_logger(console);
    spdlog::set_pattern("%^%L%$ [%H:%M:%S %z] (%n) > %v");
}

int main(void) {
    setup_logger();
    spdlog::get("server")->info("Server starting up...");

    Server s;
    s.bind(37549);
    s.run();
}