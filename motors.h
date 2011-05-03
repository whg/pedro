
#ifndef _MOTORS
#define _MOTORS

#include "main.h"
#include <stdint.h>

uint8_t stepCount1, stepCount2;

void moveHalfStep(char motor);
void changeStep(char motor, char p1, char p2, char p3, char p4);

void sleepMotors(void); //don't need to have two of these
void wakeMotors(void);  //but i think it makes things clearer

#endif