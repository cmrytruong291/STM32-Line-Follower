#ifndef PIF_CONFIG_H
#define PIF_CONFIG_H
#define FW_MINOR 8
#define PIF_SENSORS 1
#define PIF_WHEELS  2
#define PIF_TRACK   3
#ifndef PIF_LESSON
#define PIF_LESSON PIF_TRACK
#endif
#define TEST_DUTY 230
#define WHEEL_DUTY_CAP 600
#define PACK_FULLSCALE_MV 14190U  /* 33k / 10k; 33k / 8.25k: 16500U */
#define MIN_SPAN 220
#define CAL_SAMPLES 64U
#define SENSOR_MM 8.0f
#define DT_S 0.002f
#define STRAIGHT_DUTY 280
#define CORNER_DUTY 185
#ifndef KP
#define KP 13.0f
#endif
#ifndef KD
#define KD 0.12f
#endif
#ifndef KI
#define KI 0.0f
#endif
/* All frame counters below use the 2 ms TIM3 period. */
#define GAP_STOP_FRAMES   15U
#define RELOCK_FRAMES      2U
#define BAR_ENTER_FRAMES   4U
#define BAR_EXIT_FRAMES    4U
#define BAR_TIMEOUT_FRAMES 250U
#define MAX_D_MM_S         1400.0f
#if PIF_LESSON < 1 || PIF_LESSON > 3
#error "Choose PIF_SENSORS, PIF_WHEELS or PIF_TRACK"
#endif

#if CORNER_DUTY <= 0 || CORNER_DUTY > STRAIGHT_DUTY || STRAIGHT_DUTY > WHEEL_DUTY_CAP || WHEEL_DUTY_CAP > 1000 || TEST_DUTY > WHEEL_DUTY_CAP
#error "Check PWM bounds in config.h"
#endif
#endif
