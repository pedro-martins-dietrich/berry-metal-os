#pragma once

#include <cstdint>

// 240x240 IPS display driver
namespace st7789
{
    constexpr uint16_t DISPLAY_WIDTH = 240;
    constexpr uint16_t DISPLAY_HEIGHT = 240;

    // ST7789 configuration data
    struct Config
    {
        uint32_t data_command_gpio_num = 0xFF;
        uint32_t reset_gpio_num = 0xFF;
        uint32_t backlight_gpio_num = 0xFF;
        uint8_t rotation = 0; // Amount of 90 degrees rotations (0, 1, 2 or 3)
        bool invert = true;
        bool bgr_color_format = false;
    };

    // Sets up the ST7789 device - requires SPI to be configured
    bool configure(const Config& configuration);

    // Draws a rectangle with a solid color
    void fill_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    // Draws the provided image at the specified location
    void draw_bitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* pixels);
    // Changes the pixel value at the specified location
    void set_pixel(uint16_t x, uint16_t y, uint16_t color);
    // Changes all pixels from the screen to the specified color
    inline void fill_screen(uint16_t color) { fill_rectangle(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, color); }
}
