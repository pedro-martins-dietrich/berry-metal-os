#pragma once

#include <cstdint>

// Communication method with the VideoCore IV
namespace mailbox
{
    // ID of the available clocks
    typedef enum
    {
        CLOCK_ID_EMMC = 1U,
        CLOCK_ID_UART = 2U,
        CLOCK_ID_ARM = 3U,
        CLOCK_ID_CORE = 4U,
        CLOCK_ID_V3D = 5U,
        CLOCK_ID_H264 = 6U,
        CLOCK_ID_ISP = 7U,
        CLOCK_ID_SDRAM = 8U,
        CLOCK_ID_PIXEL = 9U,
        CLOCK_ID_PWM = 10U,
        CLOCK_ID_HEVC = 11U,
        CLOCK_ID_EMMC2 = 12U,
        CLOCK_ID_M2MC = 13U,
        CLOCK_ID_PIXEL_BVB = 14U
    } clock_id_t;

    // Fetches the current clock frequency
    uint32_t get_clock_frequency(clock_id_t clock_id);
}
