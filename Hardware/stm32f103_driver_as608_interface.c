#include <driver_as608_interface.h>
#include <stm32f10x.h>
#include <string.h>
#include <delay.h>

#define UART_MAX_LEN (1024)
uint8_t g_uart_rx_buffer[UART_MAX_LEN];
volatile uint16_t g_uart_point = 0;

uint8_t as608_interface_uart_init(void)
{
    // 开启串口1时钟 和 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // GPIO A9 复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // GPIO A10 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 串口1配置
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART1, ENABLE);

    return 0;
}

uint8_t as608_interface_uart_deinit(void)
{
     // 关闭 USART1
    USART_Cmd(USART1, DISABLE);

    // 禁用 USART1 中断
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 关闭 USART1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
    return 0;  
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t byte = USART_ReceiveData(USART1);
        g_uart_rx_buffer[g_uart_point++] = byte;
    }
}

uint16_t as608_interface_uart_read(uint8_t *buf, uint16_t len)
{
    uint16_t read_len;
    uint16_t g_uart_point_old;
    
    /* check receiving */
start:
    g_uart_point_old = g_uart_point;
    delay_ms(1);
    if (g_uart_point > g_uart_point_old)
    {
        goto start;
    }
    
    /* calculate the read length */
    read_len = (len < g_uart_point) ? len : g_uart_point;
    
    /* copy the data */
    memcpy(buf, g_uart_rx_buffer, read_len);
    
    /* if having remain buffer */
    if (g_uart_point > len)
    {
        /* copy remain data to the header */
        memmove(g_uart_rx_buffer, g_uart_rx_buffer + len, g_uart_point - len);
    }
    
    /* buffer point - read_len */
    g_uart_point -= read_len;
    
    return read_len;
}

uint8_t as608_interface_uart_write(uint8_t *buf, uint16_t len)
{
    for (int i = 0; i < len; i++) {
        USART_SendData(USART1, buf[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
    return 0;
}

uint8_t as608_interface_uart_flush(void)
{
    g_uart_point = 0;
    return 0;
}

void as608_interface_delay_ms(uint32_t ms)
{
    delay_ms(ms);
}

