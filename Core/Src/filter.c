#include "filter.h"
static uint16_t older[8], newer[8];
static bool have_history;

/* Median of three samples at one eye. A lone spike cannot move its reading. */
static uint16_t median3(uint16_t a0, uint16_t b, uint16_t c) {
    uint16_t low = a0 < b ? a0 : b;
    uint16_t high = a0 > b ? a0 : b;
    if (c < low) return low;
    if (c > high) return high;
    return c;
}

void Filter_Reset(void) { have_history = false; }

uint16_t Filter_Sample(unsigned eye, uint16_t raw) {
    if (!have_history) older[eye] = newer[eye] = raw;
    uint16_t sample = median3(older[eye], newer[eye], raw);
    older[eye] = newer[eye];
    newer[eye] = raw;
    return sample;
}

void Filter_EndFrame(void) { have_history = true; }
