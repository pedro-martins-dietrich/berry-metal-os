#pragma once

#include "../utils/utils.hpp"

// SPI0 driver for SPI communication
namespace spi
{
    // SPI0 configuration data
    struct Config
    {
        uint32_t clock_frequency = 8'000'000U; // Intended clock for the SPI
        bool clock_phase = false; // True means first clock transition at the beginning of data bit
        bool clock_polarity = false; // True means the rest state of the clock is high
        bool use_miso = false;
        bool chip_select_0 = false;
        bool chip_select_1 = false;
    };

    // Sets up the SPI as specified
    bool configure(const Config& config);

    // Transmits and receives data with the same size, through the SPI0
    bool transfer(const uint8_t* out_data, uint8_t* in_data, size_t size);
}
