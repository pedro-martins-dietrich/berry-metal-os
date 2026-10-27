#pragma once

#include "../gpio/gpio.hpp"
#include "../communication_protocols/uart.hpp"

// Logger for errors and debug information
namespace log
{
    // Possible levels for log messages
    enum class Level : uint8_t
    {
        Verbose,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    // Configures the error LED as an output alternative, when other methods are not available
    void configure_error_led(uint32_t gpio_num);
    // Configures log output though the UART
    void configure_log_uart(const uart::Config& uart_config);

    // Blinks the configured error LED for the specified amount of time
    void led_error(uint32_t error_code);
    // Logs the message to the configured output
    void log(Level log_level, const char* message, ...);
}
