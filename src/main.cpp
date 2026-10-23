#include "utils.hpp"
#include "gpio/gpio.hpp"
#include "uart/uart.hpp"

constexpr uint32_t LED_PIN = 17U;

void error(uint32_t code);

extern "C" void main()
{
    gpio::select(LED_PIN, gpio::MODE_OUTPUT);

    uart::Config uart_config{};
    uart_config.baud_rate = 115200U;
    uart_config.word_length = uart::WordLength::Bits8;
    uart_config.parity = uart::Parity::None;
    uart_config.stop_bits = uart::StopBits::One;
    uart_config.use_fifo = true;

    bool success = uart::configure(uart_config);
    if(!success)
        error(7U);

    uart::flushRX();

    uint8_t received_data;
    uart::ResultRX result;

    uart::print("UART Test.\n");

    while(true)
    {
        if(uart::receive(received_data, 100000U, result))
        {
            if(result != uart::ResultRX::Ok)
                error(static_cast<uint32_t>(result));
            if(received_data >= 'a' && received_data <= 'z')
                received_data -= 0x20;
            uart::send(received_data);
        }
    }

    gpio::set(LED_PIN);
    while(true);
}

void error(uint32_t code)
{
    while(true)
    {
        for(uint32_t i = 0; i < code; i++)
        {
            gpio::set(LED_PIN);
            time::wait_microseconds(700000U);

            gpio::clear(LED_PIN);
            time::wait_microseconds(300000U);
        }
        time::wait_microseconds(4000000U);
    }
}
