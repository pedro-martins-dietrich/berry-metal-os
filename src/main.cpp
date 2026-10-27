#include "utils/log.hpp"
#include "communication_protocols/spi.hpp"

constexpr uint32_t LED_PIN = 17U;

extern "C" void main()
{
    log::configure_error_led(LED_PIN);

    uart::Config uart_config{};
    uart_config.baud_rate = 115200U;
    uart_config.word_length = uart::WordLength::Bits8;
    uart_config.parity = uart::Parity::None;
    uart_config.stop_bits = uart::StopBits::One;
    uart_config.use_fifo = true;

    log::configure_log_uart(uart_config);

    spi::Config spi_config{};
    spi_config.clock_frequency = 8000000U;
    spi_config.clock_phase = true;
    spi_config.clock_polarity = true;
    spi_config.use_miso = false;
    spi_config.chip_select_0 = true;
    spi_config.chip_select_1 = false;

    if(!spi::configure(spi_config))
        log::log(log::Level::Critical, "Failed to initialize SPI.");
    log::log(log::Level::Info, "SPI successfully configured.");

    gpio::set(LED_PIN);
    while(true);
}
