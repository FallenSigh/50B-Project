#include <oled.h>
#include <oled_data.h>
#include <math.h>
#include <stm32f10x.h>

uint8_t _oled_buffer[8][128];

uint32_t QuickPow(uint32_t a, uint32_t b)
{
    uint32_t result = 1;
    while (b) {
        if (b & 1) result *= a;
        a *= a;
        b >>= 1;
    }
    return result;
}

// OLED模块的IIC总线控制引脚
void OLED_W_SCL(uint8_t x)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x));
}

void OLED_W_SDA(uint8_t x)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x));
}

uint8_t OLED_R_SDA()
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
}

uint8_t OLED_R_SCL()
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
}

// GPIO初始化
void OLED_GPIO_Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef a;
    a.GPIO_Pin   = GPIO_Pin_8;
    a.GPIO_Mode  = GPIO_Mode_Out_OD;
    a.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &a);
    a.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &a);

    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

void IIC_Start()
{
    // SDA线高电平时，SCL线高电平时，SDA线由高电平转为低电平，即启动信号
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);

    // 为下一步数据传输做准备
    OLED_W_SCL(0);
}

void IIC_Stop()
{
    // 在时钟线SCL为低电平状态下，将数据线SDA设置为低电平。
    // 保持数据线SDA为低电平，将时钟线SCL设置为高电平。
    // 在时钟线SCL为高电平状态下，将数据线SDA设置为高电平。
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

void IIC_Sendbyte(uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        OLED_W_SCL(0);
        if (byte & 0x80)
            OLED_W_SDA(1);
        else
            OLED_W_SDA(0);

        OLED_W_SCL(1);
        byte <<= 1;
    }
    OLED_W_SCL(0);
}

uint8_t IIC_Receivebyte()
{
    uint8_t byte = 0;
    OLED_W_SDA(1);
    for (int i = 0; i < 8; i++) {
        OLED_W_SCL(1);
        byte <<= 1;
        if (OLED_R_SDA()) byte |= 1;
        OLED_W_SCL(0);
    }
    return byte;
}

uint8_t IIC_WaitAck()
{
    OLED_W_SCL(1);
    uint8_t ack = OLED_R_SDA();
    OLED_W_SCL(0);
    return ack;
}

void IIC_SendAck(uint8_t ack)
{
    OLED_W_SCL(0);
    OLED_W_SDA(ack);

    OLED_W_SCL(1);
    OLED_W_SCL(0);
    OLED_W_SDA(1);
}

void OLED_WriteCommand(uint8_t command)
{
    IIC_Start();
    IIC_Sendbyte(0x78);
    IIC_WaitAck();
    IIC_Sendbyte(0x00);
    IIC_WaitAck();
    IIC_Sendbyte(command);
    IIC_WaitAck();
    IIC_Stop();
}

void OLED_WriteData(uint8_t *data, uint8_t size)
{
    IIC_Start();
    IIC_Sendbyte(0x78);
    IIC_WaitAck();
    IIC_Sendbyte(0x40);
    IIC_WaitAck();
    for (int i = 0; i < size; i++) {
        IIC_Sendbyte(data[i]);
        IIC_WaitAck();
    }
    IIC_Stop();
}

void OLED_Init()
{
    OLED_GPIO_Init();

    OLED_WriteCommand(0xAE); // 设置显示开启/关闭，0xAE关闭，0xAF开启

    OLED_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80); // 0x00~0xFF

    OLED_WriteCommand(0xA8); // 设置多路复用率
    OLED_WriteCommand(0x3F); // 0x0E~0x3F

    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00); // 0x00~0x7F

    OLED_WriteCommand(0x40); // 设置显示开始行，0x40~0x7F

    OLED_WriteCommand(0xA1); // 设置左右方向，0xA1正常，0xA0左右反置

    OLED_WriteCommand(0xC8); // 设置上下方向，0xC8正常，0xC0上下反置

    OLED_WriteCommand(0xDA); // 设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81); // 设置对比度
    OLED_WriteCommand(0xCF); // 0x00~0xFF

    OLED_WriteCommand(0xD9); // 设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB); // 设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4); // 设置整个显示打开/关闭

    OLED_WriteCommand(0xA6); // 设置正常/反色显示，0xA6正常，0xA7反色

    OLED_WriteCommand(0x8D); // 设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF); // 开启显示

    OLED_Clear();
    OLED_Update();
}

void OLED_Clear()
{
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 128; j++) {
            _oled_buffer[i][j] = 0;
        }
    }
}

void OLED_SetCursor(uint8_t page, uint8_t x)
{
    OLED_WriteCommand(0xB0 | page);              // 设置页位置
    OLED_WriteCommand(0x10 | ((x & 0xF0) >> 4)); // 设置X位置高4位
    OLED_WriteCommand(0x00 | (x & 0x0F));        // 设置X位置低4位
}

void OLED_Update()
{
    for (uint8_t i = 0; i < 8; i++) {
        OLED_SetCursor(i, 0);
        OLED_WriteData(_oled_buffer[i], 128);
    }
}

void OLED_Reverse()
{
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 128; j++) {
            _oled_buffer[i][j] = ~_oled_buffer[i][j];
        }
    }
}

void OLED_ClearArea(uint8_t x, uint8_t y, uint32_t w, uint32_t h)
{
    if (x > 128) return;
    if (y > 64) return;

    if (x + w > 128) w = 128 - x;
    if (y + h > 64) h = 64 - y;

    for (uint32_t i = y / 8; i < (y + h) / 8; i++) {
        for (uint32_t j = x; j < w + x; j++) {
            _oled_buffer[i][j] &= ~(0xff << (y % 8));
            if (i + 1 <= 7)
                _oled_buffer[i + 1][j] &= ~(0xff >> (8 - y % 8));
        }
    }
}

void OLED_ShowImage(uint8_t x, uint8_t y, uint32_t w, uint32_t h, const uint8_t *data)
{
    if (x > 128) return;
    if (y > 64) return;

    if (x + w > 128) w = 128 - x;
    if (y + h > 64) h = 64 - y;
    OLED_ClearArea(x, y, w, h);

    for (uint32_t i = y / 8; i < (y + h) / 8; i++) {
        for (uint32_t j = x; j < w + x; j++) {
            _oled_buffer[i][j] |= data[(i - y / 8) * w + j - x] << (y % 8);
            if (i + 1 <= 7)
                _oled_buffer[i + 1][j] |= data[(i - y / 8) * w + j - x] >> (8 - y % 8);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t ch, OLED_FSIZE size)
{

    switch (size) {
        case OLED_6x8:
            OLED_ShowImage(x, y, 6, 8, OLED_F6x8[ch - ' ']);
            break;
        case OLED_8x16:
            OLED_ShowImage(x, y, 8, 16, OLED_F8x16[ch - ' ']);
            break;
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, OLED_FSIZE size)
{
    for (uint16_t i = 0; str[i] != '\0'; i++) {
        OLED_ShowChar(x, y, str[i], size);
        x += size;
    }
}

uint32_t GetNumberCount(uint32_t x, uint32_t base)
{
    uint32_t cnt = 0;
    while (x) {
        x /= base;
        cnt++;
    }
    return cnt;
}

void OLED_ShowNumber(uint8_t x, uint8_t y, int num, OLED_FSIZE size)
{
    if (num < 0) {
        OLED_ShowChar(x, y, '-', size);
        x += size;
        num = -num;
    }

    if (num == 0) {
        OLED_ShowChar(x, y, '0', size);
        return;
    }

    uint32_t len = GetNumberCount(num, 10);
    while (num) {
        OLED_ShowChar(x + (len - 1) * size, y, num % 10 + '0', size);
        num /= 10;
        len--;
    }
}

void OLED_ShowHexNumber(uint8_t x, uint8_t y, int num, OLED_FSIZE size)
{
    if (num < 0) {
        OLED_ShowChar(x, y, '-', size);
        x += size;
        num = -num;
    }

    uint32_t len = GetNumberCount(num, 16);
    OLED_ShowChar(x + len * size, y, 'h', size);
    while (num) {
        uint8_t ch = num % 16;
        if (ch < 10)
            ch += '0';
        else
            ch += 'A' - 10;
        OLED_ShowChar(x + (len - 1) * size, y, ch, size);
        num /= 16;
        len--;
    }
}

void OLED_ShowBinNumber(uint8_t x, uint8_t y, int num, OLED_FSIZE size)
{
    if (num < 0) {
        OLED_ShowChar(x, y, '-', size);
        x += size;
        num = -num;
    }

    uint32_t len = GetNumberCount(num, 2);
    OLED_ShowChar(x + len * size, y, 'b', size);
    while (num) {
        OLED_ShowChar(x + (len - 1) * size, y, num % 2 + '0', size);
        num /= 2;
        len--;
    }
}

void OLED_DrawPoint(uint8_t x, uint8_t y)
{
    if (x > 128) return;
    if (y > 64) return;

    _oled_buffer[y / 8][x] |= 1 << (y % 8);
}

void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    if (x1 > 128) return;
    if (y1 > 64) return;
    if (x2 > 128) return;
    if (y2 > 64) return;

    if (x1 == x2) {
        for (uint8_t i = y1; i <= y2; i++) {
            OLED_DrawPoint(x1, i);
        }
    } else if (y1 == y2) {
        for (uint8_t i = x1; i <= x2; i++) {
            OLED_DrawPoint(i, y1);
        }
    } else {
        float k = (float)(y2 - y1) / (x2 - x1);
        float b = y1 - k * x1;
        if (fabs(k) < 1) {
            for (uint8_t i = x1; i <= x2; i++) {
                uint8_t j = k * i + b;
                OLED_DrawPoint(i, j);
            }
        } else {
            for (uint8_t i = y1; i <= y2; i++) {
                uint8_t j = (i - b) / k;
                OLED_DrawPoint(j, i);
            }
        }
    }
}

void OLED_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    OLED_DrawLine(x1, y1, x2, y1);
    OLED_DrawLine(x1, y1, x1, y2);
    OLED_DrawLine(x2, y1, x2, y2);
    OLED_DrawLine(x1, y2, x2, y2);
}

void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3)
{
    OLED_DrawLine(x1, y1, x2, y2);
    OLED_DrawLine(x1, y1, x3, y3);
    OLED_DrawLine(x2, y2, x3, y3);
}

void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
    int a, b;
    int di;
    a  = 0;
    b  = r;
    di = 3 - 2 * r;
    while (a <= b) {
        OLED_DrawPoint(x + a, y - b);
        OLED_DrawPoint(x - a, y - b);
        OLED_DrawPoint(x + a, y + b);
        OLED_DrawPoint(x - a, y + b);
        OLED_DrawPoint(x + b, y - a);
        OLED_DrawPoint(x - b, y - a);
        OLED_DrawPoint(x + b, y + a);
        OLED_DrawPoint(x - b, y + a);
        if (di < 0) {
            di += 4 * a + 6;
        } else {
            di += 4 * (a - b) + 10;
            b--;
        }
        a++;
    }
}