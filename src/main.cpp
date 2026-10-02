#include "gpio/gpio.hpp"

static void delay()
{
    for(volatile uint64_t i = 0UL; i < 400000UL; i++)
        asm volatile("nop");
}

extern "C" void main()
{
    uint32_t led_pin = 17U;
    gpio::select(led_pin, gpio::MODE_OUTPUT);

    while(true)
    {
        gpio::set(led_pin);
        delay();

        gpio::clear(led_pin);
        delay();
    }
}
