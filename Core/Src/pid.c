#include "pid.h"
#include "config.h"
#include <stdbool.h>

static bool has_prev;
static float prev_e, d_filt;
static float integral;

static float clamp(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

void Pid_Reset(void) {
    has_prev = false;
    d_filt = 0;
    integral = 0;
}

float Pid_Rate(float error) {
    float rate = has_prev ? (error - prev_e) / DT_S : 0;
    d_filt += 0.20f * (clamp(rate, -MAX_D_MM_S, MAX_D_MM_S) - d_filt);
    prev_e = error;
    has_prev = true;

    return d_filt;
}

float Pid_Turn(float error, float limit) {
    float pd = KP*error + KD*d_filt;
    if (KI > 0 && ((pd + integral < limit && error > 0) ||
                   (pd + integral > -limit && error < 0)))
        integral = clamp(integral + KI*error*DT_S, -limit, limit);
    return clamp(pd + integral, -limit, limit);
}
