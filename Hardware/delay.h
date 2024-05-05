#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

void SysTick_Init();
void delay_ms(uint32_t ms);
uint32_t get_ticks();

#ifdef __cplusplus
}
#endif

#endif