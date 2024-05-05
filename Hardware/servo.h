#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <cstdint>

class Servo
{
private:
    Servo();

public:
    static Servo& GetInstance();
    void SetAngle(uint8_t angle) const;
};

#endif