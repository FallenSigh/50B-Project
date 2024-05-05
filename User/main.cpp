#include <oled.h>
#include <delay.h>
#include <as608.h>
#include <servo.h>
#include <driver_as608.h>
#include <stm32f10x.h>

extern "C" void as608_interface_debug_print(const char *const fmt, ...)
{
    OLED_ShowString(0, 50, fmt, OLED_6x8);
    OLED_Update();
}

void a_callback(uint16_t status, const char *const fmt, ...)
{
	OLED_ClearArea(0, 10, 128, 10);
    OLED_ShowNumber(0, 10, status, OLED_6x8);
    OLED_ShowString(25, 10, fmt, OLED_6x8);
    OLED_Update();
}

void watchdog_init()
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_128);
	IWDG_SetReload(1562); // 5000ms
	IWDG_ReloadCounter();
	IWDG_Enable();
}

int main(void)
{
    OLED_Init();
	
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) {
		OLED_ShowChar(100, 50, 'W', OLED_8x16);
		OLED_Update();
		RCC_ClearFlag();
	}

    SysTick_Init();
	watchdog_init();
	auto servo = Servo::GetInstance();

	uint8_t ok = as608_basic_init(0XFFFFFFFF);
	IWDG_ReloadCounter();
	
    if (ok == 0) {
        OLED_ShowString(0, 0, "  <- Fingerprint ->", OLED_6x8);
		OLED_DrawCircle(120, 58, 4);;
        OLED_Update();
    } else {
        OLED_ShowString(0, 0, "AS608 Init Failed", OLED_6x8);
        OLED_Update();
        delay_ms(10000);
		IWDG_ReloadCounter();
    }

    while (1) {
		// feed watchdog
		IWDG_ReloadCounter();

        uint16_t page, score;
        as608_status_t status;
        // search
		uint8_t res = as608_basic_high_speed_verify(&a_callback, &page, &score, &status);

		OLED_ClearArea(0, 20, 128, 30);
        OLED_ShowString(0, 20, "res", OLED_6x8);
        OLED_ShowNumber(30, 20, res, OLED_6x8);
        OLED_ShowString(0, 30, "page", OLED_6x8);
        OLED_ShowNumber(40, 30, page, OLED_6x8);
        OLED_ShowString(0, 40, "score", OLED_6x8);
        OLED_ShowNumber(50, 40, score, OLED_6x8);
        OLED_Update();

        if (score >= 50) {
			servo.SetAngle(180);
			delay_ms(1000);
			servo.SetAngle(0);
			delay_ms(1000);
		}

    }

    return 0;
}