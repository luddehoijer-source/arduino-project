#include <stdint.h>
#include "system_sam3x.h"
#include "at91sam3x8.h"
#include "display.h"
#include "temp.h"
#include "adc.h"
#include "pwm.h"


// Styrsignal & LED
#define OE_KEY_BUS   AT91C_PIO_PD2   // PIN27
#define LED_PIN      AT91C_PIO_PD3   // PIN28

#define ROW_1        AT91C_PIO_PC5   // PIN37
#define ROW_2        AT91C_PIO_PC4   // PIN36
#define ROW_3        AT91C_PIO_PC3   // PIN35
#define ROW_4        AT91C_PIO_PC2   // PIN34

#define COL_1        AT91C_PIO_PC7   // PIN39
#define COL_2        AT91C_PIO_PC8   // PIN40
#define COL_3        AT91C_PIO_PC9   // PIN41

#define ALL_COLS     (COL_1 | COL_2 | COL_3)
#define ALL_ROWS     (ROW_1 | ROW_2 | ROW_3 | ROW_4)

static const uint32_t row_pin[4] = {ROW_1, ROW_2, ROW_3, ROW_4};
static const uint32_t col_pin[3] = {COL_1, COL_2, COL_3};

static const char key_char[13] = {
    0,
    '1', '2', '3',
    '4', '5', '6',
    '7', '8', '9',
    '*', '0', '#'
};

volatile uint32_t g_ms = 0;
volatile uint32_t delay_count = 0;
volatile char last_key = 0;

void SysTick_Handler(void) {
    g_ms++;
    if (delay_count > 0) {
        delay_count--;
    }
}

void Delay(uint32_t ms) {
    delay_count = ms + 1;
    while (delay_count > 0);
}

void keypad_init(void) {
    *AT91C_PMC_PCER = (1u << AT91C_ID_PIOD) | (1u << AT91C_ID_PIOC); // Starta klockorna

    *AT91C_PIOD_PER = OE_KEY_BUS | LED_PIN;            
    *AT91C_PIOD_SODR = OE_KEY_BUS;                      
    *AT91C_PIOD_OER = OE_KEY_BUS | LED_PIN;          
    *AT91C_PIOD_PPUDR = OE_KEY_BUS | LED_PIN;          

    *AT91C_PIOC_PER = ALL_ROWS | ALL_COLS;             
    *AT91C_PIOC_ODR = ALL_ROWS | ALL_COLS;             
    *AT91C_PIOC_PPUDR = ALL_ROWS | ALL_COLS;      

    *AT91C_PIOD_SODR = LED_PIN;                         
}

int keypad_scan(void) {
    int key = 0;

    *AT91C_PIOD_CODR = OE_KEY_BUS;                      
    *AT91C_PIOC_SODR = ALL_COLS;                        
    *AT91C_PIOC_OER = ALL_COLS;                         

    for (int Col = 0; Col < 3; Col++) {
        *AT91C_PIOC_CODR = col_pin[Col];                
        Delay(1);                                      
        uint32_t Rows = *AT91C_PIOC_PDSR;               

        for (int Row = 0; Row < 4; Row++) {
            if ((Rows & row_pin[Row]) == 0) {           // Kontrollera om raden dragits låg
                key = Row * 3 + Col + 1;
            }
        }
        *AT91C_PIOC_SODR = col_pin[Col];                
    }

    *AT91C_PIOC_ODR = ALL_COLS;                         
    *AT91C_PIOD_SODR = OE_KEY_BUS;                      

    return key;
}

char keypad_get(void) {
    static int last = 0;
    static int reported = 0;

    int now = keypad_scan();

    if (now != last) {
        last = now;
        return 0;
    }
    if (now == reported) {
        return 0;
    }

    reported = now;
    return key_char[now];
}

void handle_key(char c) {
    // Hantera knapptryckningen
    last_key = c;
    display_goto(0, 5);
    
    display_putc(c);
}

 void main(void) {
    SystemInit();
    SysTick_Config(SystemCoreClock / 1000u); // Konfigurera SysTick till 1 ms
    
    keypad_init();
    Init_Display();
    display_clear();
    
    temp_init();
    adc_init();
    pwm_init();

    temp_start();
   
    uint32_t last_scan = g_ms;
    uint32_t last_temp_ms = g_ms;
    uint32_t last_adc_ms  = g_ms;
    uint32_t last_key_ms  = g_ms;

    while (1) {

       if (temp_ready) {
         temp_ready = false; 
         int32_t temp_c10 = ((int32_t)temp_counts - 57362) / 21;  
         display_goto(0, 0);  
         display_print("Temp: ");
 
         if (temp_c10 < 0) {      
           display_putc('-');      
           temp_c10 = -temp_c10;
         }
            
         if (temp_c10 >= 1000) {     
           display_putc((char)('0' + (temp_c10 / 1000) % 10)); 
         }

         display_putc((char)('0' + (temp_c10 / 100) % 10)); 
         display_putc((char)('0' + (temp_c10 / 10) % 10)); 
         display_putc('.');
         display_putc((char)('0' + (temp_c10 % 10))); 
         display_print(" C  ");
       }
       
       if ((g_ms - last_temp_ms) >= 1000u) {   
         last_temp_ms = g_ms;  
         temp_start();
       }
       
       if ((g_ms - last_adc_ms) >= 200u) {
         last_adc_ms = g_ms;
         uint16_t raw = adc_read();
         uint32_t mv = ((uint32_t)raw * 3300u) / 4095u;
         display_goto(2, 0);
         display_print("Volt: ");
         display_putc((char)('0' + (mv / 1000u) % 10u));
         display_putc('.');
         display_putc((char)('0' + (mv / 100u) % 10u));
         display_putc((char)('0' + (mv / 10u) % 10u));
         display_print(" V");
       }
       
       if ((g_ms - last_key_ms) >= 10u) {   
         last_key_ms = g_ms;
         char key = keypad_get();
       }
         if (key != 0) {
                
           display_goto(6, 0);
                
           display_print("Key: ");
                
           display_putc(key);
         }
         if (key >= '1' && key <= '9') {
                
           uint16_t angle = (uint16_t)(key - '0') * 10u;  
           uint16_t pulse_us = 700u + (angle * 1600u) / 180u;
           servo_set(pulse_us);  
           display_goto(4, 0); 
           display_print("Servo: ");              
           display_putc((char)('0' + (angle / 10u)));  
           display_print("0 deg");
         }
      }
   }

