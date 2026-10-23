#pragma once

#include <cstddef>
#include <cstdint>

// Creates a reference to the specified address location
inline volatile uint32_t& reg(uintptr_t address)
{
    return *reinterpret_cast<volatile uint32_t*>(address);
}

namespace time
{
    constexpr uintptr_t SYSTEM_CLOCK = 0x3F003004;

    // Fetches the current microsecond from the system clock (1us resolution)
    inline uint32_t get_microsecond()
    {
        return reg(SYSTEM_CLOCK);
    }

    // Waits for the specified time (in microseconds)
    inline void wait_microseconds(uint32_t us)
    {
        const uint32_t t0 = reg(SYSTEM_CLOCK);
        while(reg(SYSTEM_CLOCK) - t0 <= us);
    }
}
