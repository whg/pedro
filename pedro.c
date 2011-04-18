

///////////////////////////////
// - - pedro - - - - - - - - //
///////////////////////////////

#include "pedro.h"


int main (void) {
	
	initUSART();
	initMotors();
	initTimers();
	sei();
	
	//set up postion
//	pos.x = 0;
//	pos.y = 0;
	
	nextLineFlag = 0;
		
	LED_DDR |= (1<<LED_PIN);
	LED_PORT |= (1<<LED_PIN);
	
	while (1) {
		
		
		if (nextLineFlag) {
			line(A.x, A.y, B.x, B.y);
		}
		
	}
	
	return 0;
	
}

// INTERRUPT FUNCTIONS

ISR(USART_RXC_vect) {
		
	rxBuffer[rxbc] = UDR;
	rxbc++;
	if (rxbc == RX_BUFFERSIZE) {
		processUSART();
		//setMotors();
		rxbc = 0;
	}
	
}

// MAIN FUNCTIONS

void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	
	int16_t steep = (abs(y1-y0) > abs(x1 -x0));
	int8_t xstep = 1;
	int8_t ystep = 1;
	
	if (y0 > y1) {
		ystep = -1;
	}
	
	if (x0 > x1) {
		xstep = -1;
	}

	
	if (steep) {
		swap16(&x0, &y0);
		swap16(&x1, &y1);
	}
	
	if (x0 > x1) {
		swap16(&x0, &x1);
	}
	

	
	int16_t dx = (int16_t) x1 - x0;
	int16_t dy = (int16_t) abs(y1 - y0);
	int16_t err = (int16_t) dx/2;
	int16_t y = (int16_t) y0;
	
	wakeMotors();

	uint8_t send = 0;
	
	uint16_t x;
	for (x = x0; x < x1; x++) {
		
		//move x
		if (steep) {
			moveHalfStep(1, stepCount1);
			stepCount1+= xstep;
		} else {
			moveHalfStep(2, stepCount2);
			stepCount2+= ystep;
		}
		
		err-= dy;
		
		//move y
		if (err < 0) {	
			if (steep) {
				moveHalfStep(2, stepCount2);
				stepCount2+= ystep;
			} else {
				moveHalfStep(1, stepCount1);
				stepCount1+= xstep;
			}			
			y+= ystep;
			err+= dx;
		}
		
		//delay once...
		delay_ms(delayTime);
		
		//send for new coordinates
		//do this once, halfway through the line
		if (x > (dx/2) && !send) {
			UDR = 56;
			send = 1;
		}
		
		//update pos
		pos.x = x;
		pos.y = y;
		
	}
	
	if (x1 == B.x && y1 == B.y) {
		//this means we have reached out destination.
		nextLineFlag = 0;
	}
	
	sleepMotors();
	
}



void processUSART(void) {
	
	switch (rxBuffer[0]) {
			
		//change delay time
		case 0:
			delayTime = rxBuffer[1];
			break;
			
		//single line
		case 1:
			line((rxBuffer[1] | (rxBuffer[2]<<8)), (rxBuffer[3] | (rxBuffer[4]<<8)),
					 (rxBuffer[5] | (rxBuffer[6]<<8)), (rxBuffer[7] | (rxBuffer[8]<<8)));
			break;
			
		//multiple lines
		case 2:
			//assume we have multiple lines
			nextLineFlag = 1;
						
			//set point A and B
			A.x = (rxBuffer[1] | (rxBuffer[2]<<8));
			A.y = (rxBuffer[3] | (rxBuffer[4]<<8));
			B.x = (rxBuffer[5] | (rxBuffer[6]<<8));
			B.y = (rxBuffer[7] | (rxBuffer[8]<<8));
			break;
		
		//move
		case 3:
			line(pos.x, pos.y, (rxBuffer[1] | (rxBuffer[2]<<8)), (rxBuffer[3] | (rxBuffer[4]<<8)));
			break;

		//get pos, send pos coordinates
		case 4:
			while ((UCSRA & (1 << UDRE)) == 0) {};
			UDR = pos.x;
			while ((UCSRA & (1 << UDRE)) == 0) {};
			UDR = (pos.x>>8);
			while ((UCSRA & (1 << UDRE)) == 0) {};
			UDR = pos.y;
			while ((UCSRA & (1 << UDRE)) == 0) {};
			UDR = (pos.y>>8);
			break;

			
	}// end switch
	
	
}

void setMotors(void) { }

void sleepMotors(void) {
	M1_PORT &= ~(1<<M1_ENABLE);
	M2_PORT &= ~(1<<M2_ENABLE);
}

void wakeMotors(void) {
	M1_PORT |= (1<<M1_ENABLE);
	M2_PORT |= (1<<M2_ENABLE);
}


//this function moves a specified motor half a step
void moveHalfStep(char motor, int8_t stepCount) {
	
	
	switch (stepCount) {
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
	
	//reset the step counts, if necessary
	//this is not very nicely done...
	if (stepCount1 == 8) {
		stepCount1 = 0;
	} else if (stepCount1 == -1) {
		stepCount1 = 7;
	} if (stepCount2 == 8) {
		stepCount2 = 0;
	} else if (stepCount2 == -1){
		stepCount2 = 7;
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


void delay_ms(int d) {	
	while (d) {
		_delay_ms(0.25);
		d--;
	}
}

// UTIL FUNCTIONS

void swap16(uint16_t *a, uint16_t *b) {
	
	uint16_t t = *a;
	*a = *b;
	*b = t;
}


// INIT FUNCTIONS

void initUSART(void) {
	//enable USART transmissions and reception
	UCSRB|= (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);
      //use 8 bit character size
	UCSRC |= (1<<UCSZ1) | (1<<UCSZ0);
	
	UBRRL = BAUD_PRESCALE;
	//load upper 8 bits to high register
	UBRRH = (BAUD_PRESCALE>>8);
	
}

void initMotors(void) {
	
	//set up all pins
	M1_DDR  |= (1<<M1_PIN1) | (1<<M1_PIN2) | (1<<M1_PIN3) | (1<<M1_PIN4) | (1<<M1_ENABLE);
	M1_PORT |= (1<<M1_PIN1) | (1<<M1_PIN2) | (1<<M1_PIN3) | (1<<M1_PIN4);
	
	M2_DDR  |= (1<<M2_PIN1) | (1<<M2_PIN2) | (1<<M2_PIN3) | (1<<M2_PIN4) | (1<<M2_ENABLE);
	M2_PORT |= (1<<M2_PIN1) | (1<<M2_PIN2) | (1<<M2_PIN3) | (1<<M2_PIN4);
	
	//sleepmotors
	M1_PORT &= ~(1<<M1_ENABLE);
	M2_PORT &= ~(1<<M2_ENABLE);
	
	delayTime = 20;
}

void initTimers(void) {
	
	//set timer0 to prescale of 1024
	TCCR0 |= (1<<CS00) | (1<<CS02);
	
	//set timer2 to prescale of 1024
	TCCR2 |= (1<<CS22) | (1<<CS21) | (1<<CS20);
}

