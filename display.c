#include "display.h"
#include "system_sam3x.h"
#include "at91sam3x8.h"

// Deklarera
extern void Delay(uint32_t ms);

// PC2 - PC9
#define DATA_BUS        (0xFFu << 2)


#define DISP_OE         AT91C_PIO_PC12  // PIN51
#define DISP_DIR        AT91C_PIO_PC13  // PIN50
#define DISP_CD         AT91C_PIO_PC14  // PIN49
#define DISP_CE         AT91C_PIO_PC15  // PIN48
#define DISP_RD         AT91C_PIO_PC16  // PIN47
#define DISP_WR         AT91C_PIO_PC17  // PIN46
#define DISP_FS         AT91C_PIO_PC18  // PIN45 
#define DISP_RV         AT91C_PIO_PC19  // PIN44

#define DISP_CTRL_PIOC  (DISP_OE | DISP_DIR | DISP_CD | DISP_CE | DISP_RD | DISP_WR | DISP_FS | DISP_RV)


#define DISP_RESET      AT91C_PIO_PD0   

// Skärmdimensioner
#define DISP_COLS       30u
#define DISP_ROWS       16u

unsigned char Read_Status_Display(void) {
    unsigned char temp;
    *AT91C_PIOC_ODR = DATA_BUS;

    // Vänd transceiver till läsning, sen aktivera
    *AT91C_PIOC_SODR = DISP_DIR;
    *AT91C_PIOC_CODR = DISP_OE;

    *AT91C_PIOC_SODR = DISP_CD;
    *AT91C_PIOC_CODR = DISP_CE | DISP_RD;
    Delay(1);

    // Läs av bussen medan CE och RD fortfarande är låga
    temp = (unsigned char)((*AT91C_PIOC_PDSR & DATA_BUS) >> 2);

    // Avslutar och nollställer för att lämna bussen fri
    *AT91C_PIOC_SODR = DISP_CE | DISP_RD;
    *AT91C_PIOC_SODR = DISP_OE;
    *AT91C_PIOC_CODR = DISP_DIR;

    return temp;
}

static void Wait_Display_Ready(void) {
    uint32_t timeout = 100000u;
    while (((Read_Status_Display() & 0x03u) != 0x03u) && (timeout > 0u)) {
        timeout--;
    }
}

void Write_Command_2_Display(unsigned char Command) {
    Wait_Display_Ready();

    // Förladda databussen medan pinnarna fortfarande är ingångar, OE hög
    *AT91C_PIOC_CODR = DATA_BUS;
    *AT91C_PIOC_SODR = ((uint32_t)Command << 2);

    // Ställ transceiver för skrivning och aktivera utgångar
    *AT91C_PIOC_CODR = DISP_DIR;
    *AT91C_PIOC_CODR = DISP_OE;
    *AT91C_PIOC_OER  = DATA_BUS;

 
    *AT91C_PIOC_SODR = DISP_CD;
    *AT91C_PIOC_CODR = DISP_CE | DISP_WR;

    Delay(1);

    // Avsluta skrivning
    *AT91C_PIOC_SODR = DISP_CE | DISP_WR;
    *AT91C_PIOC_SODR = DISP_OE;
    *AT91C_PIOC_ODR  = DATA_BUS;
}

void Write_Data_2_Display(unsigned char Data) {
    Wait_Display_Ready();

    // Laddar in 
    *AT91C_PIOC_CODR = DATA_BUS;
    *AT91C_PIOC_SODR = ((uint32_t)Data << 2);

    
    *AT91C_PIOC_CODR = DISP_DIR;
    *AT91C_PIOC_CODR = DISP_OE;
    *AT91C_PIOC_OER  = DATA_BUS;

    
    *AT91C_PIOC_CODR = DISP_CD;
    *AT91C_PIOC_CODR = DISP_CE | DISP_WR;

    Delay(1);

    
    *AT91C_PIOC_SODR = DISP_CE | DISP_WR;
    *AT91C_PIOC_SODR = DISP_OE;
    *AT91C_PIOC_ODR  = DATA_BUS;
}

void Init_Display(void) {
   
    *AT91C_PMC_PCER = (1u << AT91C_ID_PIOC) | (1u << AT91C_ID_PIOD);

   
    *AT91C_PIOC_PER = DATA_BUS | DISP_CTRL_PIOC;
    *AT91C_PIOD_PER = DISP_RESET;

    
    *AT91C_PIOC_ODR   = DATA_BUS;
    *AT91C_PIOC_PPUDR = DATA_BUS;

    // Vilolägen
    *AT91C_PIOC_SODR = DISP_OE | DISP_CE | DISP_RD | DISP_WR | DISP_CD;
    *AT91C_PIOC_CODR = DISP_DIR | DISP_FS | DISP_RV;
    *AT91C_PIOC_OER  = DISP_CTRL_PIOC;

    // Resettar
    *AT91C_PIOD_CODR = DISP_RESET;
    *AT91C_PIOD_OER  = DISP_RESET;
    Delay(10);
    *AT91C_PIOD_SODR = DISP_RESET;
    Delay(10);

    
    Write_Data_2_Display(0x00);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x40);

   
    Write_Data_2_Display(0x00);
    Write_Data_2_Display(0x40);
    Write_Command_2_Display(0x42);

    
    Write_Data_2_Display(0x1E);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x41);

     
    Write_Data_2_Display(0x1E);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x43);
      
    
    Write_Command_2_Display(0x80);

    //  Text ON, Graphic OFF
    Write_Command_2_Display(0x94);
}

void display_goto(uint8_t row, uint8_t col) {
    uint16_t address = (uint16_t)row * DISP_COLS + (uint16_t)col;
    Write_Data_2_Display((unsigned char)(address & 0xFFu));
    Write_Data_2_Display((unsigned char)(address >> 8));
    Write_Command_2_Display(0x24); // Set Address Pointer
}

void display_putc(char c) {
    // (ASCII 0x20 till 0x00)
    Write_Data_2_Display((unsigned char)(c - 0x20));
    Write_Command_2_Display(0xC0); 
}

void display_print(const char *text) {
    while (*text != '\0') {
        display_putc(*text);
        text++;
    }
}

void display_clear(void) {
    display_goto(0, 0);
    for (uint16_t i = 0; i < (DISP_ROWS * DISP_COLS); i++) {
        display_putc(' '); // Skriver 0x00 till hela textminnet
    }
    display_goto(0, 0);
}