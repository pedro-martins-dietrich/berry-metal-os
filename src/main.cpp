#include "utils/log.hpp"
#include "communication_protocols/spi.hpp"
#include "peripherals/st7789.hpp"

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

    st7789::Config st7789_config{};
    st7789_config.data_command_gpio_num = 25U;
    st7789_config.reset_gpio_num = 24U;
    st7789_config.backlight_gpio_num = 23U;
    st7789_config.rotation = 0;
    st7789_config.invert = true;
    st7789_config.bgr_color_format = false;

    if(!st7789::configure(st7789_config))
        log::log(log::Level::Critical, "Failed to initialize ST7789 device.");
    log::log(log::Level::Info, "ST7789 device successfully initialized.");

    st7789::fill_rectangle(40, 0, 40, 120, 0x001F);
    st7789::fill_rectangle(120, 160, 120, 40, 0xFFFF);

    gpio::set(LED_PIN);
    while(true);
}
