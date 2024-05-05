#include "delay.h"
#include "stm32f10x.h"

uint32_t ms_ticks;

void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while (1);  
    }
}

void SysTick_Handler(void)
{
    ms_ticks++;
}

void delay_ms(uint32_t ms)
{
    uint32_t start_ms = ms_ticks;
    while (ms_ticks - start_ms < ms);
}

uint32_t get_ticks()
{
    return ms_ticks;
}
