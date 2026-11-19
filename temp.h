// temp.h
#ifndef TEMP_H
#define TEMP_H

#include <stdint.h>
#include <stdbool.h>

extern volatile bool temp_ready;
extern volatile uint32_t temp_counts;

void temp_init(void);
void temp_start(void);

#endif // TEMP_H