#include "uart.hpp"

#include "../utils.hpp"
#include "../gpio/gpio.hpp"

namespace uart
{
    constexpr uintptr_t BASE = 0x3F201000;
    constexpr uintptr_t DATA = BASE;
    constexpr uintptr_t STATUS = BASE + 0x04;
    constexpr uintptr_t FLAG = BASE + 0x18;
    constexpr uintptr_t INTEGER_BAUD_RATE_DIVISOR = BASE + 0x24;
    constexpr uintptr_t FRACTIONAL_BAUD_RATE_DIVISOR = BASE + 0x28;
    constexpr uintptr_t LINE_CONTROL = BASE + 0x2C;
    constexpr uintptr_t CONTROL = BASE + 0x30;
    constexpr uintptr_t INTERRUPT_FIFO_LEVEL_SELECT = BASE + 0x34;
    constexpr uintptr_t INTERRUPT_MASK_SET_CLEAR = BASE + 0x38;
    constexpr uintptr_t RAW_INTERRUPT_STATUS = BASE + 0x3C;
    constexpr uintptr_t MASKED_INTERRUPT_STATUS = BASE + 0x40;
    constexpr uintptr_t INTERRUPT_CLEAR = BASE + 0x44;
    constexpr uintptr_t DMA_CONTROL = BASE + 0x48;
    constexpr uintptr_t TEST_CONTROL = BASE + 0x80;
    constexpr uintptr_t INTEGRATION_TEST_INPUT = BASE + 0x84;
    constexpr uintptr_t INTEGRATION_TEST_OUTPUT = BASE + 0x88;
    constexpr uintptr_t TEST_DATA = BASE + 0x8C;

    constexpr uint32_t TX_PIN = 14U;
    constexpr uint32_t RX_PIN = 15U;

    // Flags raised by the PL011
    typedef enum
    {
        FLAG_CLEAR_TO_SEND = 1U,
        FLAG_BUSY = 1U << 3U,
        FLAG_RX_FIFO_EMPTY = 1U << 4U,
        FLAG_TX_FIFO_FULL = 1U << 5U,
        FLAG_RX_FIFO_FULL = 1U << 6U,
        FLAG_TX_FIFO_EMPTY = 1U << 7U
    } flag_t;

    // UART line control configuration parameters
    typedef enum
    {
        LINE_CONTROL_BREAK = 1U,
        LINE_CONTROL_PARITY_ENABLE = 1U << 1U,
        LINE_CONTROL_EVEN_PARITY_SELECT = 1U << 2U,
        LINE_CONTROL_TWO_STOP_BITS_SELECT = 1U << 3U,
        LINE_CONTROL_FIFO_ENABLE = 1U << 4U,
        LINE_CONTROL_WORD_LENGTH_5_BITS = 0b00 << 5U,
        LINE_CONTROL_WORD_LENGTH_6_BITS = 0b01 << 5U,
        LINE_CONTROL_WORD_LENGTH_7_BITS = 0b10 << 5U,
        LINE_CONTROL_WORD_LENGTH_8_BITS = 0b11 << 5U,
        LINE_CONTROL_STICK_PARITY_SELECT = 1U << 7U
    } line_control_t;

    // UART control configuration parameters
    typedef enum
    {
        CONTROL_ENABLE = 1U,
        CONTROL_LOOPBACK_ENABLE = 1U << 7U,
        CONTROL_TX_ENABLE = 1U << 8U,
        CONTROL_RX_ENABLE = 1U << 9U,
        CONTROL_REQUEST_TO_SEND = 1U << 11U,
        CONTROL_REQUEST_TO_SEND_ENABLE = 1U << 14U,
        CONTROL_CLEAR_TO_RECEIVE_ENABLE = 1U << 15U
    } control_t;

    // UART data error bits
    typedef enum
    {
        DATA_FRAMING_ERROR = 1U << 8U,
        DATA_PARITY_ERROR = 1U << 9U,
        DATA_BREAK_ERROR = 1U << 10U,
        DATA_OVERRUN_ERROR = 1U << 11U
    } data_error_t;

    static bool configured = false;
    static bool loopback_mode = false;
    static uint32_t clock = 48'000'000U;
    static uint8_t data_mask = 0xFF;

    // Updates the PL011 control
    static void update_control();
    // Configures the UART to the specified baud rate
    static bool set_baud_div(uint32_t baud_rate);
}

bool uart::configure(Config configuration)
{
    if(reg(CONTROL) & CONTROL_ENABLE)
        flushTX();

    reg(CONTROL) = 0U;
    reg(LINE_CONTROL) = 0U;

    gpio::select(TX_PIN, gpio::MODE_ALT_0);
    gpio::select(RX_PIN, gpio::MODE_ALT_0);

    reg(INTERRUPT_MASK_SET_CLEAR) = 0U;
    reg(INTERRUPT_CLEAR) = 0x7FF;

    bool status = set_baud_div(configuration.baud_rate);
    if(!status) return false;

    uint32_t line_control_config = 0U;
    if(configuration.use_fifo)
        line_control_config |= LINE_CONTROL_FIFO_ENABLE;
    if(configuration.stop_bits == StopBits::Two)
        line_control_config |= LINE_CONTROL_TWO_STOP_BITS_SELECT;
    switch(configuration.word_length)
    {
        case WordLength::Bits5:
            line_control_config |= LINE_CONTROL_WORD_LENGTH_5_BITS;
            data_mask = 0x1F;
            break;
        case WordLength::Bits6:
            line_control_config |= LINE_CONTROL_WORD_LENGTH_6_BITS;
            data_mask = 0x3F;
            break;
        case WordLength::Bits7:
            line_control_config |= LINE_CONTROL_WORD_LENGTH_7_BITS;
            data_mask = 0x7F;
            break;
        case WordLength::Bits8:
            line_control_config |= LINE_CONTROL_WORD_LENGTH_8_BITS;
            data_mask = 0xFF;
            break;
    }
    switch(configuration.parity)
    {
        case Parity::None:
            break;
        case Parity::Odd:
            line_control_config |= LINE_CONTROL_PARITY_ENABLE;
            break;
        case Parity::Even:
            line_control_config |= LINE_CONTROL_PARITY_ENABLE | LINE_CONTROL_EVEN_PARITY_SELECT;
            break;
        case Parity::Always0:
            line_control_config |= LINE_CONTROL_PARITY_ENABLE
                | LINE_CONTROL_EVEN_PARITY_SELECT | LINE_CONTROL_STICK_PARITY_SELECT;
            break;
        case Parity::Always1:
            line_control_config |= LINE_CONTROL_PARITY_ENABLE | LINE_CONTROL_STICK_PARITY_SELECT;
            break;
    }
    reg(LINE_CONTROL) = line_control_config;

    update_control();

    configured = true;
    return true;
}

void uart::set_loopback(bool loopback_enable)
{
    loopback_mode = loopback_enable;
    if(reg(CONTROL) && CONTROL_ENABLE)
        update_control();
}

void uart::send(const uint8_t* data, size_t size)
{
    while(size--)
        send(*data++);
}

void uart::send(uint8_t data)
{
    while(reg(FLAG) & FLAG_TX_FIFO_FULL);
    reg(DATA) = data;
}

void uart::print(const char* message)
{
    if(!configured || message == nullptr) return;

    while(*message)
    {
        if(*message == '\n')
            send('\r');
        send(static_cast<uint8_t>(*message++));
    }
}

void uart::flushTX()
{
    while(reg(FLAG) & FLAG_BUSY);
}

uart::ResultRX uart::try_receive(uint8_t& data)
{
    if(reg(FLAG) & FLAG_RX_FIFO_EMPTY)
        return ResultRX::Empty;

    const uint32_t read_data = reg(DATA);
    data = static_cast<uint8_t>(read_data) & data_mask;

    if(read_data & (DATA_FRAMING_ERROR | DATA_PARITY_ERROR | DATA_BREAK_ERROR | DATA_OVERRUN_ERROR))
    {
        reg(STATUS) = 0U;
        if(read_data & DATA_FRAMING_ERROR) return ResultRX::FramingError;
        if(read_data & DATA_PARITY_ERROR) return ResultRX::ParityError;
        if(read_data & DATA_BREAK_ERROR) return ResultRX::BreakError;
        return ResultRX::OverrunError;
    }
    return ResultRX::Ok;
}

bool uart::receive(uint8_t& data, uint32_t timeout_us, ResultRX& status)
{
    const uint32_t t0 = time::get_microsecond();
    while(time::get_microsecond() - t0 <= timeout_us)
    {
        status = try_receive(data);
        if(status != ResultRX::Empty)
            return true;
    }

    status = ResultRX::Empty;
    return false;
}

void uart::flushRX()
{
    uint8_t temp;
    while(try_receive(temp) != ResultRX::Empty);
}

bool uart::checkRX()
{
    return !(reg(FLAG) & FLAG_RX_FIFO_EMPTY);
}

void uart::update_control()
{
    uint32_t control_configuration = CONTROL_ENABLE | CONTROL_RX_ENABLE | CONTROL_TX_ENABLE;
    if(loopback_mode)
        control_configuration |= CONTROL_LOOPBACK_ENABLE;
    reg(CONTROL) = control_configuration;
}

bool uart::set_baud_div(uint32_t baud_rate)
{
    if(baud_rate == 0U || baud_rate > (clock >> 4U))
        return false;

    uint32_t baud_div_times_64 = ((static_cast<uint64_t>(clock) << 2U) + (baud_rate >> 1U)) / baud_rate;
    reg(INTEGER_BAUD_RATE_DIVISOR) = baud_div_times_64 >> 6U;
    reg(FRACTIONAL_BAUD_RATE_DIVISOR) = baud_div_times_64 & 0x3F;

    return true;
}
