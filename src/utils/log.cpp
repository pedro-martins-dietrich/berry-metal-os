#include "log.hpp"

#include "utils.hpp"
#include "format.hpp"

namespace log
{
    bool error_led_configured = false;
    bool uart_configured = false;

    uint32_t error_led_gpio_num = 0U;
}

void log::configure_error_led(uint32_t gpio_num)
{
    gpio::select(gpio_num, gpio::MODE_OUTPUT);
    error_led_gpio_num = gpio_num;
    error_led_configured = true;
}

void log::configure_log_uart(const uart::Config& uart_config)
{
    if(!uart::configure(uart_config))
    {
        led_error(1U);
        return;
    }
    uart_configured = true;
    log(Level::Info, "UART log configured.\n");
}

void log::led_error(uint32_t error_code)
{
    while(error_led_configured)
    {
        for(uint32_t i = 0U; i < error_code; i++)
        {
            gpio::set(error_led_gpio_num);
            time::wait_microseconds(500000U);

            gpio::clear(error_led_gpio_num);
            time::wait_microseconds(200000U);
        }
        time::wait_microseconds(4000000U);
    }
}

void log::log(Level log_level, const char* message, ...)
{
    if(!uart_configured)
    {
        led_error(1U);
        return;
    }

    const char* prefix = "[UNKNOWN] ";
    switch(log_level)
    {
        case Level::Verbose:
            prefix = "[VERBOSE] ";
            break;
        case Level::Debug:
            prefix = "[DEBUG] ";
            break;
        case Level::Info:
            prefix = "[INFO] ";
            break;
        case Level::Warning:
            prefix = "[WARNING] ";
            break;
        case Level::Error:
            prefix = "[ERROR] ";
            break;
        case Level::Critical:
            prefix = "[CRITICAL] ";
            break;
    }

    char buffer[256];
    va_list args;
    va_start(args, message);
    format::vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);
    uart::print(prefix);
    uart::print(buffer);
    uart::print("\n");

    if(log_level == Level::Critical)
        led_error(2U);
}
