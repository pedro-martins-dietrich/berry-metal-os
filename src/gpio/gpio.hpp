#pragma once

#include <cstdint>

// General Purpose Input-Output
namespace gpio
{
    constexpr uintptr_t BASE = 0x3F200000;
    constexpr uintptr_t SELECT = BASE;
    constexpr uintptr_t SET = BASE + 0x1C;
    constexpr uintptr_t CLEAR = BASE + 0x28;
    constexpr uintptr_t READ = BASE + 0x34;
    constexpr uintptr_t EVENT_DETECT = BASE + 0x40;
    constexpr uintptr_t RISING_EDGE_DETECT = BASE + 0x4C;
    constexpr uintptr_t FALLING_EDGE_DETECT = BASE + 0x58;
    constexpr uintptr_t HIGH_LEVEL_DETECT = BASE + 0x64;
    constexpr uintptr_t LOW_LEVEL_DETECT = BASE + 0x70;
    constexpr uintptr_t ASYNC_RISING_EDGE_DETECT = BASE + 0x7C;
    constexpr uintptr_t ASYNC_FALLING_EDGE_DETECT = BASE + 0x88;
    constexpr uintptr_t PULL_UP_DOWN = BASE + 0x94;
    constexpr uintptr_t PULL_UP_DOWN_CLOCK = BASE + 0x98;

    constexpr uint32_t MAX_GPIO_NUMBER = 54U;

    // Available GPIO pin modes
    typedef enum
    {
        MODE_INPUT = 0b000,
        MODE_OUTPUT = 0b001,
        MODE_ALT_5 = 0b010,
        MODE_ALT_4 = 0b011,
        MODE_ALT_0 = 0b100,
        MODE_ALT_1 = 0b101,
        MODE_ALT_2 = 0b110,
        MODE_ALT_3 = 0b111
    } select_mode_t;

    // Selects the mode for the specified GPIO pin
    void select(uint32_t gpio_num, select_mode_t mode);
    // Sets the value for the specified GPIO pin
    void set(uint32_t gpio_num);
    // Clears the value for the specified GPIO pin
    void clear(uint32_t gpio_num);
    // Reads and returns the value of the specified pin.
    bool read(uint32_t gpio_num);
}
