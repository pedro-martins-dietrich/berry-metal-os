#include "st7789.hpp"

#include "../gpio/gpio.hpp"
#include "../communication_protocols/spi.hpp"

namespace st7789
{
    // Commands accepted by the ST7789
    typedef enum : uint8_t
    {
        COMMAND_NOP = 0x00,
        COMMAND_SOFTWARE_RESET = 0x01,
        COMMAND_READ_DISPLAY_ID = 0x04,
        COMMAND_READ_DISPLAY_STATUS = 0x09,
        COMMAND_READ_DISPLAY_POWER = 0x0A,
        COMMAND_READ_DISPLAY_MAD_CONTROL = 0x0B,
        COMMAND_READ_DISPLAY_PIXEL_FORMAT = 0x0C,
        COMMAND_READ_DISPLAY_IMAGE_MODE = 0x0D,
        COMMAND_READ_DISPLAY_SIGNAL_MODE = 0x0E,
        COMMAND_READ_DISPLAY_SELF_DIAGNOSTIC_RESULT = 0x0F,
        COMMAND_SLEEP_IN = 0x10,
        COMMAND_SLEEP_OUT = 0x11,
        COMMAND_PARTIAL_DISPLAY_MODE_ON = 0x12,
        COMMAND_NORMAL_DISPLAY_MODE_ON = 0x13,
        COMMAND_DISPLAY_INVERSION_OFF = 0x20,
        COMMAND_DISPLAY_INVERSION_ON = 0x21,
        COMMAND_DISPLAY_OFF = 0x28,
        COMMAND_DISPLAY_ON = 0x29,
        COMMAND_COLUMN_ADDRESS_SET = 0x2A,
        COMMAND_ROW_ADDRESS_SET = 0x2B,
        COMMAND_RAM_WRITE = 0x2C,
        COMMAND_RAM_READ = 0x2E,
        COMMAND_PARTIAL_AREA = 0x30,
        COMMAND_MEMORY_ACCESS_DATA_CONTROL = 0x36,
        COMMAND_VERTICAL_SCROLL_START_ADDRESS = 0x37,
        COMMAND_IDLE_MODE_OFF = 0x38,
        COMMAND_IDLE_MODE_ON = 0x39,
        COMMAND_INTERFACE_PIXEL_FORMAT = 0x3A
    } command_t;

    // Available pixel formats for the display
    typedef enum : uint8_t
    {
        COLOR_MODE_12_BPP = 0x04,
        COLOR_MODE_16_BPP = 0x05,
        COLOR_MODE_18_BPP = 0x06,
        COLOR_MODE_16M_TRUNCATED = 0x07,
        COLOR_MODE_RGB_65K = 0x50,
        COLOR_MODE_RGB_262K = 0x60
    } interface_pixel_format_t;

    // Modes for screen orientation, refresh order and pixel color order
    typedef enum : uint8_t
    {
        MAD_CONTROL_BOTTOM_TO_TOP = 0x80,
        MAD_CONTROL_RIGHT_TO_LEFT = 0x40,
        MAD_CONTROL_TRANSPOSE_XY = 0x20,
        MAD_CONTROL_REFRESH_BOTTOM_TO_TOP = 0x10,
        MAD_CONTROL_BGR = 0x08,
        MAD_CONTROL_REFRESH_RIGHT_TO_LEFT = 0x04,
        MAD_CONTROL_NONE = 0x00
    } memory_access_data_control_t;

    constexpr size_t chunk_size = 512;

    static Config config;
    static bool configured = false;
    static bool ok_status = false;

    static uint8_t x_offset = 0;
    static uint8_t y_offset = 0;

    // Sends a command and the following data to the display
    void send_command(command_t command, const uint8_t* data = nullptr, size_t size = 0);
    // Streams data to the display
    void send_data(const uint8_t* data, size_t size);

    // Configures the screen rotation
    void set_rotation();
    // Defines the working window
    void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
}

bool st7789::configure(const Config& configuration)
{
    config = configuration;

    gpio::select(config.data_command_gpio_num, gpio::MODE_OUTPUT);
    gpio::select(config.backlight_gpio_num, gpio::MODE_OUTPUT);
    gpio::select(config.reset_gpio_num, gpio::MODE_OUTPUT);

    gpio::set(config.data_command_gpio_num);
    gpio::clear(config.backlight_gpio_num);

    gpio::set(config.reset_gpio_num);
    time::wait_microseconds(10'000U);
    gpio::clear(config.reset_gpio_num);
    time::wait_microseconds(20'000U);
    gpio::set(config.reset_gpio_num);
    time::wait_microseconds(120'000U);

    send_command(COMMAND_SOFTWARE_RESET);
    time::wait_microseconds(150'000U);
    send_command(COMMAND_SLEEP_OUT);
    time::wait_microseconds(120'000U);

    const uint8_t pixel_format = COLOR_MODE_16_BPP | COLOR_MODE_RGB_65K;
    send_command(COMMAND_INTERFACE_PIXEL_FORMAT, &pixel_format, 1);
    time::wait_microseconds(10'000U);

    set_rotation();
    send_command(config.invert ? COMMAND_DISPLAY_INVERSION_ON : COMMAND_DISPLAY_INVERSION_OFF);
    send_command(COMMAND_NORMAL_DISPLAY_MODE_ON);
    time::wait_microseconds(20'000U);

    fill_screen(0x0000);
    send_command(COMMAND_DISPLAY_ON);
    time::wait_microseconds(20'000U);

    gpio::set(config.backlight_gpio_num);
    return ok_status;
}

void st7789::fill_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    if(!configured || width == 0 || height == 0) return;
    if(x + width > DISPLAY_WIDTH)
        width = DISPLAY_WIDTH - x;
    if(y + height > DISPLAY_HEIGHT)
        height = DISPLAY_HEIGHT - y;

    set_window(x, y, x + width - 1, y + height - 1);

    uint8_t pixel_data[chunk_size];
    for(size_t i = 0; i < chunk_size; i += 2)
    {
        pixel_data[i] = static_cast<uint8_t>(color >> 8);
        pixel_data[i + 1] = static_cast<uint8_t>(color & 0xFF);
    }
    uint32_t remaining_bytes = 2U * width * height;
    while(remaining_bytes)
    {
        const uint32_t buffer_size = (remaining_bytes < chunk_size) ? remaining_bytes : chunk_size;
        send_data(pixel_data, buffer_size);
        remaining_bytes -= buffer_size;
    }
}

void st7789::draw_bitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* pixels)
{
    if(!configured || !pixels || width == 0 || height == 0) return;
    if(x + width > DISPLAY_WIDTH)
        width = DISPLAY_WIDTH - x;
    if(y + height > DISPLAY_HEIGHT)
        height = DISPLAY_HEIGHT - y;

    set_window(x, y, x + width - 1, y + height - 1);

    uint8_t pixel_data[chunk_size];
    uint32_t remaining_bytes = 2U * width * height;
    while(remaining_bytes)
    {
        const uint32_t buffer_size = (remaining_bytes < chunk_size) ? remaining_bytes : chunk_size;
        for(uint32_t i = 0; i < buffer_size; i += 2)
        {
            pixel_data[i] = static_cast<uint8_t>(pixels[i] >> 8);
            pixel_data[i + 1] = static_cast<uint8_t>(pixels[i] & 0xFF);
        }
        send_data(pixel_data, buffer_size);
        pixels += (buffer_size >> 1);
        remaining_bytes -= buffer_size;
    }
}

void st7789::set_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(!configured || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    set_window(x, y, x, y);
    const uint8_t pixel_data[2] = {uint8_t(color >> 8), uint8_t(color & 0xFF)};
    send_data(pixel_data, 2);
}

void st7789::send_command(command_t command, const uint8_t* data, size_t size)
{
    gpio::clear(config.data_command_gpio_num);
    if(!spi::transfer(reinterpret_cast<uint8_t*>(&command), nullptr, 1))
        ok_status = false;
    if(size)
        send_data(data, size);
}

void st7789::send_data(const uint8_t* data, size_t size)
{
    gpio::set(config.data_command_gpio_num);
    if(!spi::transfer(data, nullptr, size))
        ok_status = false;
}

void st7789::set_rotation()
{
    uint8_t memory_access_data_control = MAD_CONTROL_NONE;
    switch(config.rotation & 0x03)
    {
        case 0x00:
            memory_access_data_control = MAD_CONTROL_NONE;
            x_offset = 0;
            y_offset = 0;
            break;
        case 0x01:
            memory_access_data_control = MAD_CONTROL_RIGHT_TO_LEFT | MAD_CONTROL_TRANSPOSE_XY;
            x_offset = 0;
            y_offset = 0;
            break;
        case 0x02:
            memory_access_data_control = MAD_CONTROL_RIGHT_TO_LEFT | MAD_CONTROL_BOTTOM_TO_TOP;
            x_offset = 0;
            y_offset = 80;
            break;
        case 0x03:
            memory_access_data_control = MAD_CONTROL_BOTTOM_TO_TOP | MAD_CONTROL_TRANSPOSE_XY;
            x_offset = 80;
            y_offset = 0;
            break;
    }
    memory_access_data_control |= (config.bgr_color_format ? MAD_CONTROL_BGR : MAD_CONTROL_NONE);
    send_command(COMMAND_MEMORY_ACCESS_DATA_CONTROL, &memory_access_data_control, 1);
}

void st7789::set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    x0 += x_offset;
    x1 += x_offset;
    y0 += y_offset;
    y1 += y_offset;

    const uint8_t column_set[4] = {uint8_t(x0 >> 8), uint8_t(x0 & 0xFF), uint8_t(x1 >> 8), uint8_t(x1 & 0xFF)};
    const uint8_t row_set[4] = {uint8_t(y0 >> 8), uint8_t(y0 & 0xFF), uint8_t(y1 >> 8), uint8_t(y1 & 0xFF)};

    send_command(COMMAND_COLUMN_ADDRESS_SET, column_set, 4);
    send_command(COMMAND_ROW_ADDRESS_SET, row_set, 4);
    send_command(COMMAND_RAM_WRITE);
}
