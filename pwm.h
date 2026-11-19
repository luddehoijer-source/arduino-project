#ifndef PWM_H
#define PWM_H

#include <stdint.h>

void pwm_init(void);
void servo_set(uint16_t pulse_us);

#endif // PWM_H