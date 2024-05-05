#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>

class PWM
{
private:
    PWM();

public:
    static PWM& GetInstance();
    void setDuty(uint16_t duty) const;
};

#endif