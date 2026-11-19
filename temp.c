// temp.c
#include "temp.h"
#include "at91sam3x8.h"
#include "system_sam3x.h"

extern void Delay(uint32_t ms);

#define TEMP_PIN          AT91C_PIO_PB25  // PIN 2 = PB25 = TIOA0
#define START_PULSE_TICKS 420u            // 10 us vid 42 MHz

volatile bool temp_ready = false;
volatile uint32_t temp_counts = 0;

void temp_init(void) {
    *AT91C_PMC_PCER = (1u << AT91C_ID_PIOB) | (1u << AT91C_ID_TC0);

    *AT91C_PIOB_PER   = TEMP_PIN;
    *AT91C_PIOB_ODR   = TEMP_PIN;
    *AT91C_PIOB_PPUDR = TEMP_PIN;

    *AT91C_TC0_CCR = AT91C_TC_CLKDIS;
    *AT91C_TC0_IDR = 0xFFFFFFFFu;
    *AT91C_TC0_CMR = (0u << 0) | (2u << 16) | (2u << 18); // TIMER_CLOCK1, LDRA/LDRB falling

    *AT91C_TC0_CCR = AT91C_TC_CLKEN;
    NVIC_ClearPendingIRQ(TC0_IRQn);
    NVIC_EnableIRQ(TC0_IRQn);

    // Reset-puls 4.6-16 ms (10 ms)
    *AT91C_PIOB_CODR = TEMP_PIN;
    *AT91C_PIOB_OER  = TEMP_PIN;
    Delay(10);
    *AT91C_PIOB_ODR  = TEMP_PIN;
    Delay(1);
}

void temp_start(void) {
    temp_ready = false;

    (void)*AT91C_TC0_SR;
    *AT91C_TC0_CCR = AT91C_TC_SWTRG;
    *AT91C_TC0_IER = AT91C_TC_LDRBS;

    uint32_t t0 = *AT91C_TC0_CV;
    *AT91C_PIOB_CODR = TEMP_PIN;
    *AT91C_PIOB_OER  = TEMP_PIN;

    while ((*AT91C_TC0_CV - t0) < START_PULSE_TICKS) {
    }

    *AT91C_PIOB_ODR = TEMP_PIN;
}

void TC0_Handler(void) {
    uint32_t status = *AT91C_TC0_SR;
    if ((status & AT91C_TC_LDRBS) != 0u) {
        *AT91C_TC0_IDR = AT91C_TC_LDRBS;
        temp_counts = *AT91C_TC0_RB - *AT91C_TC0_RA;
        temp_ready = true;
    }
}