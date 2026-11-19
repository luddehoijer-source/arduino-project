#include "pwm.h"
#include "at91sam3x8.h"

#define SERVO_PIN       AT91C_PIO_PB17 // PIN 62 (A8) = PB17 = PWML1
#define PWM_CH_SERVO    1u
#define PWM_PERIOD_20MS 52500u

void pwm_init(void) {
    *AT91C_PMC_PCER  = (1u << AT91C_ID_PIOB);
    *AT91C_PMC_PCER1 = (1u << (AT91C_ID_PWMC - 32));

    *AT91C_PIOB_PDR   = SERVO_PIN;
    *AT91C_PIOB_PPUDR = SERVO_PIN;
    *AT91C_PIOB_ABSR |= SERVO_PIN;

    *AT91C_PWMC_DIS       = (1u << PWM_CH_SERVO);
    *AT91C_PWMC_CH1_CMR   = 5u;              // MCK / 32 = 2.625 MHz
    *AT91C_PWMC_CH1_CPRDR = PWM_PERIOD_20MS; // 20 ms
    *AT91C_PWMC_CH1_CDTYR = 1838u;           // 0.7 ms (startläge)
    *AT91C_PWMC_ENA       = (1u << PWM_CH_SERVO);
}

void servo_set(uint16_t pulse_us) {
    *AT91C_PWMC_CH1_CDTYUPDR = ((uint32_t)pulse_us * 2625u) / 1000u;
}