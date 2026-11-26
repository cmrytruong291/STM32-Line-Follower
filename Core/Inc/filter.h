#ifndef PIF_FILTER_H
#define PIF_FILTER_H
#include <stdbool.h>
#include <stdint.h>
void Filter_Reset(void);
uint16_t Filter_Sample(unsigned eye, uint16_t raw);
void Filter_EndFrame(void);
#endif
