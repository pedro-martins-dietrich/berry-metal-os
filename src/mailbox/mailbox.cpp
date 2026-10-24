#include "mailbox.hpp"

#include "../utils.hpp"

namespace mailbox
{
    constexpr uintptr_t BASE = 0x3F00B880;
    constexpr uintptr_t READ = BASE;
    constexpr uintptr_t STATUS = BASE + 0x18;
    constexpr uintptr_t WRITE = BASE + 0x20;

    // Mailbox status
    typedef enum
    {
        STATUS_FULL = 1U << 31U,
        STATUS_EMPTY = 1U << 30U
    } status_t;

    // Mailbox request tags
    typedef enum
    {
        TAG_END = 0x0,
        TAG_GET_FIRMWARE_REVISION = 0x00000001,
        TAG_SET_CURSOR_INFO = 0x00008010,
        TAG_SET_CURSOR_STATE = 0x00008011,

        TAG_GET_BOARD_MODEL = 0x00010001,
        TAG_GET_BOARD_REVISION = 0x00010002,
        TAG_GET_BOARD_MAC_ADDRESS = 0x00010003,
        TAG_GET_BOARD_SERIAL_NUMBER = 0x00010004,
        TAG_GET_ARM_MEMORY = 0x00010005,
        TAG_GET_VIDEOCORE_MEMORY = 0x00010006,
        TAG_GET_CLOCKS = 0x00010007,

        TAG_GET_POWER_STATE = 0x00020001,
        TAG_GET_TIMING = 0x00020002,
        TAG_SET_POWER_STATE = 0x00028001,

        TAG_GET_CLOCK_STATE = 0x00030001,
        TAG_GET_CLOCK_RATE = 0x00030002,
        TAG_GET_VOLTAGE = 0x00030003,
        TAG_GET_MAX_CLOCK_RATE = 0x00030004,
        TAG_GET_MAX_VOLTAGE = 0x00030005,
        TAG_GET_TEMPERATURE = 0x00030006,
        TAG_GET_MIN_CLOCK_RATE = 0x00030007,
        TAG_GET_MIN_VOLTAGE = 0x00030008,
        TAG_GET_TURBO = 0x00030009,
        TAG_GET_MAX_TEMPERATURE = 0x0003000A,
        TAG_ALLOCATE_MEMORY = 0x0003000C,
        TAG_LOCK_MEMORY = 0x0003000D,
        TAG_UNLOCK_MEMORY = 0x0003000E,
        TAG_RELEASE_MEMORY = 0x0003000F,
        TAG_EXECUTE_CODE = 0x00030010,
        TAG_GET_DISPMANX_RESOURCE_MEM_HANDLE = 0x00030014,
        TAG_GET_EDID_BLOCK = 0x00030020,
        TAG_GET_CLOCK_RATE_MEASURED = 0x00030047,
        TAG_SET_CLOCK_RATE = 0x00038002,
        TAG_SET_VOLTAGE = 0x00038003,
        TAG_SET_TURBO = 0x00038009,

        TAG_ALLOCATE_FRAMEBUFFER = 0x00040001,
        TAG_FRAMEBUFFER_BLANK = 0x00040002,
        TAG_GET_SCREEN_PHYSICAL_DIMENSIONS = 0x00040003,
        TAG_GET_SCREEN_VIRTUAL_DIMENSIONS = 0x00040004,
        TAG_GET_PIXEL_DEPTH = 0x00040005,
        TAG_GET_PIXEL_ORDER = 0x00040006,
        TAG_GET_ALPHA_MODE = 0x00040007,
        TAG_GET_PITCH = 0x00040008,
        TAG_GET_VIRTUAL_OFFSET = 0x00040009,
        TAG_GET_OVERSCAN = 0x0004000A,
        TAG_GET_PALETTE = 0x0004000B,
        TAG_GET_TOUCHBUFFER = 0x0004000F,
        TAG_GET_GPIO_VIRTUAL_BUFFER = 0x00040010,
        TAG_TEST_PHYSICAL_DISPLAY = 0x00044003,
        TAG_TEST_VIRTUAL_DISPLAY = 0x00044004,
        TAG_TEST_PIXEL_DEPTH = 0x00044005,
        TAG_TEST_PIXEL_ORDER = 0x00044006,
        TAG_TEST_ALPHA_MODE = 0x00044007,
        TAG_TEST_VIRTUAL_OFFSET = 0x00044009,
        TAG_TEST_OVERSCAN = 0x0004400A,
        TAG_TEST_PALETTE = 0x0004400B,
        TAG_RELEASE_FRAMEBUFFER = 0x00048001,
        TAG_SET_SCREEN_PHYSICAL_DIMENSIONS = 0x00048003,
        TAG_SET_SCREEN_VIRTUAL_DIMENSIONS = 0x00048004,
        TAG_SET_PIXEL_DEPTH = 0x00048005,
        TAG_SET_PIXEL_ORDER = 0x00048006,
        TAG_SET_ALPHA_MODE = 0x00048007,
        TAG_SET_VIRTUAL_OFFSET = 0x00048009,
        TAG_SET_OVERSCAN = 0x0004800A,
        TAG_SET_PALETTE = 0x0004800B,
        TAG_SET_VSYNC = 0x0004800E,
        TAG_SET_BACKLIGHT = 0x0004800F,
        TAG_SET_TOUCH_BUFFER = 0x0004801F,
        TAG_SET_GPIO_VIRTUAL_BUFFER = 0x00048020,

        TAG_GET_COMMAND_LINE = 0x00050001,

        TAG_GET_DMA_CHANNELS = 0x00060001,
    } tag_t;

    typedef enum
    {
        CHANNEL_POWER = 0U,
        CHANNEL_FRAMEBUFFER = 1U,
        CHANNEL_VIRTUAL_UART = 2U,
        CHANNEL_VIDEOCORE_HOST_INTERFACE_QUEUE = 3U,
        CHANNEL_LEDS = 4U,
        CHANNEL_BUTTONS = 5U,
        CHANNEL_TOUCHSCREEN = 6U,
        CHANNEL_COUNTER = 7U,
        CHANNEL_PROPERTY = 8U
    } channel_t;

    constexpr uint32_t REQUEST_CODE = 0x0;
    constexpr uint32_t RESPONSE_OK_CODE = 0x80000000;
    constexpr uint32_t RESPONSE_ERROR_CODE = 0x80000001;

    constexpr uint32_t TIMEOUT_US = 100000;

    static alignas(16) volatile uint32_t message[8];

    // Sends the message to the GPU and awaits the answer
    bool exchange(channel_t channel);
}

uint32_t mailbox::get_clock_frequency(clock_id_t clock_id)
{
    message[0] = 8U * sizeof(uint32_t); // Buffer size (B)
    message[1] = REQUEST_CODE;
    message[2] = TAG_GET_CLOCK_RATE;
    message[3] = 0U; // Value buffer size
    message[4] = 4U; // Request size
    message[5] = clock_id;
    message[6] = 0U; // GPU's response for the clock rate, in Hz
    message[7] = TAG_END;

    return exchange(CHANNEL_PROPERTY) ? message[6] : 0U;
}

bool mailbox::exchange(channel_t channel)
{
    const uint32_t message_address = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&message[0]));
    const uint32_t mail = (message_address & ~0xF) | (channel & 0xF);

    uint32_t t0 = time::get_microsecond();
    while(reg(STATUS) & STATUS_FULL)
    {
        if(time::get_microsecond() - t0 > TIMEOUT_US)
            return false;
    }
    reg(WRITE) = mail;

    t0 = time::get_microsecond();
    while(true)
    {
        while(reg(STATUS) & STATUS_FULL)
        {
            if(time::get_microsecond() - t0 > TIMEOUT_US)
                return false;
        }
        if(reg(READ) == mail) break;
    }
    return (message[1] == RESPONSE_OK_CODE);
}
