#include "pid.h"
#include "config.h"
#include <assert.h>
#include <math.h>
int main(void) {
    Pid_Reset();
    Pid_Rate(2);
    float u = Pid_Turn(2,100);
    assert(u >= 2*KP && u <= 2*KP + 0.1f);
    for (unsigned i=0; i<5000; ++i) {
        Pid_Rate(2);
        u=Pid_Turn(2,100);
        assert(u <= 100 && u >= -100);
    }
    if (KI > 0) assert(u > 2*KP + 5);
    else assert(fabsf(u - 2*KP) < 0.001f);
    for (unsigned i=0; i<8000; ++i) {
        Pid_Rate(-5);
        u=Pid_Turn(-5,100);
    }
    assert(u < 0 && u >= -100);
    Pid_Reset();
    assert(Pid_Rate(0) == 0 && Pid_Turn(0,100) == 0);
}
