#ifndef PIF_PID_H
#define PIF_PID_H
void Pid_Reset(void);
float Pid_Rate(float error);
float Pid_Turn(float error, float limit);
#endif
