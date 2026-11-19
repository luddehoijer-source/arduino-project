#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// Grundfunktioner
unsigned char Read_Status_Display(void);
void Write_Command_2_Display(unsigned char Command);
void Write_Data_2_Display(unsigned char Data);
void Init_Display(void);

// Textfunktioner
void display_goto(uint8_t row, uint8_t col);
void display_putc(char c);
void display_print(const char *text);
void display_clear(void);

#endif