#pragma once

#include <cstddef>
#include <cstdint>

// PL011 UART driver for UART communication
namespace uart
{
    // Size in bits of a single word
    enum class WordLength : uint8_t
    {
        Bits5,
        Bits6,
        Bits7,
        Bits8
    };

    // Parity bit(s) appended to the end of a word
    enum class Parity : uint8_t
    {
        None,
        Odd,
        Even,
        Always0,
        Always1
    };

    // Number of stop bits to use
    enum class StopBits : uint8_t
    {
        One,
        Two
    };

    // Possible results for a received message
    enum class ResultRX : uint8_t
    {
        Ok,
        Empty,
        FramingError,
        ParityError,
        BreakError,
        OverrunError
    };

    // PL011 UART configuration data
    struct Config
    {
        uint32_t baud_rate = 115200U;
        WordLength word_length = WordLength::Bits8;
        Parity parity = Parity::None;
        StopBits stop_bits = StopBits::One;
        bool use_fifo = true;
    };

    // Configures the UART with the specified baud rate
    bool configure(Config configuration);
    // Enables or disables the loopback feature in the PL011
    void set_loopback(bool loopback_enable);

    // Transmists multiple bytes through the UART
    void send(const uint8_t* data, size_t size);
    // Transmists a single byte through the UART
    void send(uint8_t data);
    // Transmists a string using through the UART
    void print(const char* message);
    // Waits until the transmitter is no longer busy
    void flushTX();

    // Tries to fetch a single byte from the UART receiver
    ResultRX try_receive(uint8_t& data);
    // Waits until a byte is received from the UART, or until timeout
    bool receive(uint8_t& data, uint32_t timeout_us, ResultRX& status);
    // Waits until the receiver gets empty
    void flushRX();
    // Verifies if the receiver is clear to receive
    bool checkRX();
}
