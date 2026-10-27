#include "spi.hpp"

#include "../gpio/gpio.hpp"
#include "../mailbox/mailbox.hpp"
#include "../utils/log.hpp"

namespace spi
{
    constexpr uintptr_t SPI_0_BASE = 0x3F204000;
    constexpr uintptr_t CONTROL_STATUS = SPI_0_BASE;
    constexpr uintptr_t FIFO = SPI_0_BASE + 0x04;
    constexpr uintptr_t CLOCK_DIVIDER = SPI_0_BASE + 0x08;
    constexpr uintptr_t DATA_LENGTH = SPI_0_BASE + 0x0C;
    constexpr uintptr_t LOSSI_MODE_TURN_ON_HALFWAY = SPI_0_BASE + 0x10;
    constexpr uintptr_t DMA_DATA_REQUEST_CONTROLS = SPI_0_BASE + 0x14;

    constexpr uint32_t CHIP_SELECT_1_PIN = 7U;
    constexpr uint32_t CHIP_SELECT_0_PIN = 8U;
    constexpr uint32_t MISO_PIN = 9U;
    constexpr uint32_t MOSI_PIN = 10U;
    constexpr uint32_t SERIAL_CLOCK_PIN = 11U;

    constexpr uint32_t STALL_TIMEOUT_US = 40000U;

    // Flags used for the control/status register
    typedef enum
    {
        CONTROL_CHIP_SELECT_0 = 0b00,
        CONTROL_CHIP_SELECT_1 = 0b01,
        CONTROL_CHIP_SELECT_2 = 0b10,
        CONTROL_CLOCK_PHASE = 1U << 2U,
        CONTROL_CLOCK_POLARITY = 1U << 3U,
        CONTROL_CLEAR_TX = 1U << 4U,
        CONTROL_CLEAR_RX = 1U << 5U,
        CONTROL_CHIP_SELECT_POLARITY = 1U << 6U,
        CONTROL_TRANSFER_ACTIVE = 1U << 7U,
        CONTROL_DMA_ENABLE = 1U << 8U,
        CONTROL_INTERRUPT_ON_DONE = 1U << 9U,
        CONTROL_INTERRUPT_ON_RX_NEEDS_READING = 1U << 10U,
        CONTROL_AUTOMATICALLY_DEASSERT_CHIP_SELECT = 1U << 11U,
        CONTROL_READ_ENABLE = 1U << 12U,
        CONTROL_LOSSI_ENABLE = 1U << 13U,
        STATUS_DONE = 1U << 16U,
        STATUS_RX_HAS_DATA = 1U << 17U,
        STATUS_TX_CAN_ACCEPT_DATA = 1U << 18U,
        STATUS_RX_NEEDS_READING = 1U << 19U,
        STATUS_RX_FULL = 1U << 20U,
        CONTROL_CHIP_SELECT_POLARITY_0 = 1U << 21U,
        CONTROL_CHIP_SELECT_POLARITY_1 = 1U << 22U,
        CONTROL_CHIP_SELECT_POLARITY_2 = 1U << 23U,
        CONTROL_DMA_IN_LOSSI_MODE_ENABLE = 1U << 24U,
        CONTROL_LONG_DATA_WORD_IN_LOSSI_MODE_ENABLE = 1U << 25U
    } control_status_t;

    static uint32_t core_clock_frequency = 0U;
    static uint32_t spi_clock_frequency = 0U;
    static uint32_t configured_control = 0U;

    // Calculates and configures the SPI clock divider
    static bool set_clock_divider(uint32_t spi_clock_frequency);
}

bool spi::configure(const Config& config)
{
    core_clock_frequency = mailbox::get_clock_frequency(mailbox::CLOCK_ID_CORE);
    if(core_clock_frequency == 0U)
    {
        log::log(log::Level::Error, "Failed to fetch core clock frequency for SPI configuration.");
        return false;
    }

    reg(CONTROL_STATUS) = CONTROL_CLEAR_TX | CONTROL_CLEAR_RX;

    if(config.chip_select_0)
        gpio::select(CHIP_SELECT_0_PIN, gpio::MODE_ALT_0);
    if(config.chip_select_1)
        gpio::select(CHIP_SELECT_1_PIN, gpio::MODE_ALT_0);
    if(config.use_miso)
        gpio::select(MISO_PIN, gpio::MODE_ALT_0);
    gpio::select(MOSI_PIN, gpio::MODE_ALT_0);
    gpio::select(SERIAL_CLOCK_PIN, gpio::MODE_ALT_0);

    configured_control = 0U;
    if(config.chip_select_1)
        configured_control |= CONTROL_CHIP_SELECT_1;
    if(config.clock_polarity)
        configured_control |= CONTROL_CLOCK_POLARITY;
    if(config.clock_phase)
        configured_control |= CONTROL_CLOCK_PHASE;

    return set_clock_divider(config.clock_frequency);
}

bool spi::transfer(const uint8_t* out_data, uint8_t* in_data, size_t size)
{
    if(spi_clock_frequency == 0U) return false;
    if(size == 0) return true;

    reg(CONTROL_STATUS) = configured_control | CONTROL_CLEAR_TX | CONTROL_CLEAR_RX;
    reg(CONTROL_STATUS) = configured_control | CONTROL_TRANSFER_ACTIVE;

    size_t tx_i = 0;
    size_t rx_i = 0;
    bool ok = true;
    uint32_t last_time = time::get_microsecond();

    while(rx_i < size)
    {
        bool progress = false;
        while(tx_i < size && (reg(CONTROL_STATUS) & STATUS_TX_CAN_ACCEPT_DATA))
        {
            reg(FIFO) = out_data ? out_data[tx_i] : 0U;
            tx_i++;
            progress = true;
        }
        while(rx_i < size && (reg(CONTROL_STATUS) & STATUS_RX_HAS_DATA))
        {
            const uint8_t received_data = static_cast<uint8_t>(reg(FIFO));
            if(in_data)
                in_data[rx_i] = received_data;
            rx_i++;
            progress = true;
        }

        if(progress)
            last_time = time::get_microsecond();
        else if(time::get_microsecond() - last_time > STALL_TIMEOUT_US)
        {
            ok = false;
            break;
        }
    }

    if(ok)
    {
        const uint32_t t0 = time::get_microsecond();
        while(!(reg(CONTROL_STATUS) & STATUS_DONE))
        {
            if(time::get_microsecond() - t0 > STALL_TIMEOUT_US)
            {
                ok = false;
                break;
            }
        }
    }

    reg(CONTROL_STATUS) = configured_control | CONTROL_CLEAR_TX | CONTROL_CLEAR_RX;
    return ok;
}

bool spi::set_clock_divider(uint32_t requested_clock_frequency)
{
    if(core_clock_frequency == 0U || requested_clock_frequency == 0U)
    {
        log::log
        (
            log::Level::Error,
            "Invalid clock values for SPI configuration. Core clock: %d - Requested clock: %d.",
            core_clock_frequency, requested_clock_frequency
        );
        return false;
    }

    uint32_t divider = (core_clock_frequency + (requested_clock_frequency - 1U)) / requested_clock_frequency;
    if(divider & 1U) divider++;
    if(divider > 0x10000) return false;

    reg(CLOCK_DIVIDER) = (divider & 0xFFFF);
    spi_clock_frequency = core_clock_frequency / divider;

    return true;
}
