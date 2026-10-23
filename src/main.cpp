#include "utils.hpp"
#include "gpio/gpio.hpp"

extern "C" void main()
{
    const uint32_t led_pin = 17U;
    gpio::select(led_pin, gpio::MODE_OUTPUT);

    while(true)
    {
        gpio::set(led_pin);
        time::wait_microseconds(500U);

        gpio::clear(led_pin);
        time::wait_microseconds(500U);
    }
}
