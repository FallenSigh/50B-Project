#include <servo.h>
#include <pwm.h>
#include <stm32f10x.h>

Servo::Servo()
{
    auto pwm = PWM::GetInstance();
    
}

Servo &Servo::GetInstance()
{
    static Servo s;
    return s;
}

void Servo::SetAngle(uint8_t angle) const
{
    auto pwm = PWM::GetInstance();
    // 1000 - 5000
    pwm.setDuty(1000 + (angle * 4000 / 180));
}
