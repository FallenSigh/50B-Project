#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>
#include "oled_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    OLED_6x8  = 6,
    OLED_8x16 = 8,
} OLED_FSIZE;

void OLED_Init();
void OLED_Clear();
void OLED_SetCursor(uint8_t Page, uint8_t X);
void OLED_Update();
void OLED_Reverse();
void OLED_ClearArea(uint8_t x, uint8_t y, uint32_t w, uint32_t h);

void OLED_ShowImage(uint8_t x, uint8_t y, uint32_t w, uint32_t h, const uint8_t* data);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t ch, OLED_FSIZE size);
void OLED_ShowString(uint8_t x, uint8_t y, const char* str, OLED_FSIZE size);
void OLED_ShowNumber(uint8_t x, uint8_t y, int num, OLED_FSIZE size);
void OLED_ShowHexNumber(uint8_t x, uint8_t y, int num, OLED_FSIZE size);
void OLED_ShowBinNumber(uint8_t x, uint8_t y, int num, OLED_FSIZE size);

void OLED_DrawPoint(uint8_t x, uint8_t y);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3);
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r);

#if __cplusplus
}
#endif

#endif



