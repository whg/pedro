#include "motors.h"


//this function moves a specified motor half a step
void moveHalfStep(char motor) {
	
	uint8_t stepCount;
	
	if (motor == 1) {
		stepCount = stepCount1;
	}
	else {
		stepCount = stepCount2;
	}
	
	switch (stepCount % 8) {
		case 0:
			changeStep(motor, 1, 0, 0, 0);
			break;
		case 1:
			changeStep(motor, 1, 0, 1, 0);
			break;
		case 2:
			changeStep(motor, 0, 0, 1, 0);
			break;
		case 3:
			changeStep(motor, 0, 1, 1, 0);
			break;
		case 4:
			changeStep(motor, 0, 1, 0, 0);
			break;
		case 5:
			changeStep(motor, 0, 1, 0, 1);
			break;
		case 6:
			changeStep(motor, 0, 0, 0, 1);
			break;
		case 7:
			changeStep(motor, 1, 0, 0, 1);
			break;
	}
	
	//end reset
}

void changeStep(char motor, char p1, char p2, char p3, char p4) {
	
	switch (motor) {
		case 1:
			if (p1 == 0) M1_PORT |= (1<<M1_PIN1);
			else M1_PORT &=	~(1<<M1_PIN1);
			if (p2 == 0) M1_PORT |= (1<<M1_PIN2);
			else M1_PORT &=	~(1<<M1_PIN2);
			if (p3 == 0) M1_PORT |= (1<<M1_PIN3);
			else M1_PORT &=	~(1<<M1_PIN3);
			if (p4 == 0) M1_PORT |= (1<<M1_PIN4);
			else M1_PORT &=  ~(1<<M1_PIN4);
			break;
			
		case 2:
			if (p1 == 0) M2_PORT |= (1<<M2_PIN1);
			else M2_PORT &=	~(1<<M2_PIN1);
			if (p2 == 0) M2_PORT |= (1<<M2_PIN2);
			else M2_PORT &=	~(1<<M2_PIN2);
			if (p3 == 0) M2_PORT |= (1<<M2_PIN3);
			else M2_PORT &=	~(1<<M2_PIN3);
			if (p4 == 0) M2_PORT |= (1<<M2_PIN4);
			else M2_PORT &=  ~(1<<M2_PIN4);
			break;
			
	}
	
}

void sleepMotors(void) {
	M1_PORT &= ~(1<<M1_ENABLE);
	M2_PORT &= ~(1<<M2_ENABLE);
}

void wakeMotors(void) {
	M1_PORT |= (1<<M1_ENABLE);
	M2_PORT |= (1<<M2_ENABLE);
}

