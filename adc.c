#include "adc.h"
#include "at91sam3x8.h"

#define ADC_BASE     ((AT91PS_ADC)0x400C0000u)
#define ADC_LIGHT_CH 2u // PIN 59 (AN5) = PA4 = AD2

void adc_init(void) {
    *AT91C_PMC_PCER1   = (1u << (37 - 32)); // Peripheral ID 37 = ADC
    ADC_BASE->ADC_CR   = (1u << 0);         // SWRST (Software Reset)
    ADC_BASE->ADC_MR   = (2u << 8) | (8u << 16) | (2u << 24); // PRESCAL = 2 -> 14 MHz
    ADC_BASE->ADC_CHER = (1u << ADC_LIGHT_CH); // Aktivera kana
}

uint16_t adc_read(void) {
    ADC_BASE->ADC_CR = (1u << 1); // START conversion
    while ((ADC_BASE->ADC_SR & (1u << 24)) == 0u) { // Vänta på DRDY (bit 24
    }
    return (uint16_t)(ADC_BASE->ADC_LCDR & 0x0FFFu); // Läs 12-bitars värde
}