#include "gpio.hpp"

void gpio::select(uint32_t gpio_num, select_mode_t mode)
{
    if(gpio_num >= MAX_GPIO_NUMBER) return;

    uint32_t reg_offset = gpio_num / 10U;
    uint32_t shift_amount = 3U * (gpio_num % 10U);

    volatile uint32_t* gpio_reg_select = (volatile uint32_t*) SELECT;
    gpio_reg_select[reg_offset] &= ~(0b111 << shift_amount);
    gpio_reg_select[reg_offset] |= (mode << shift_amount);
}

void gpio::set(uint32_t gpio_num)
{
    if(gpio_num >= MAX_GPIO_NUMBER) return;

    uint32_t reg_offset = gpio_num / 32U;
    uint32_t shift_amount = gpio_num % 32U;

    ((volatile uint32_t*) SET)[reg_offset] = 1U << shift_amount;
}

void gpio::clear(uint32_t gpio_num)
{
    if(gpio_num >= MAX_GPIO_NUMBER) return;

    uint32_t reg_offset = gpio_num / 32U;
    uint32_t shift_amount = gpio_num % 32U;

    ((volatile uint32_t*) CLEAR)[reg_offset] = 1U << shift_amount;
}

bool gpio::read(uint32_t gpio_num)
{
    if(gpio_num >= MAX_GPIO_NUMBER) return false;

    uint32_t reg_offset = gpio_num / 32U;
    uint32_t shift_amount = gpio_num % 32U;

    volatile uint32_t* gpio_reg_read = (volatile uint32_t*) READ;

    return static_cast<bool>(1U & (gpio_reg_read[reg_offset] >> shift_amount));
}
