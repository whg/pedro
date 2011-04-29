

///////////////////////////////
// - - pedro - - - - - - - - //
///////////////////////////////

#include "pedro.h"


int main (void) {
	
	LED_DDR |= (1<<LED_PIN);
	LED_PORT &= ~(1<<LED_PIN);
	
	initUSART();
	initMotors();
	initPen();
	initTimers();
	
	sei();
	
	
	//set up postion
//	pos.x = 0;
//	pos.y = 0;
	
	nextLineFlag = 0;
	
		
	
	while (1) {
		
		
		if (decodeNext() == 1) {
			//got next command to process
			
			LED_PORT |= (1<<LED_PIN); //LED on
			
			switch (currentCommand.commandCode) {
				case COMMAND_CODE_PEN_UP:
					movePenUp();
					break;
				case COMMAND_CODE_PEN_DOWN:
					movePenDown();
					break;
				case COMMAND_CODE_MOVE_REL:
					moveRel(currentCommand.command.moveRel.x, currentCommand.command.moveRel.y);
					break;
				case COMMAND_CODE_MOVE_ABS:
					moveAbs(currentCommand.command.moveAbs.x, currentCommand.command.moveAbs.y);
					break;
				default:
					break;
			}
			
			
			//turn LED off
			//LED_PORT |= (1<<LED_PIN);

		}
				
		else {
			//no commands to process
			
			
			//if idle for a while turn motors off...
		}


		
//		if (nextLineFlag) {
//			LED_PORT &= ~(1<<LED_PIN);
//			
//			LastB.x = B.x;
//			LastB.y = B.y;
//			
//			line(A.x, A.y, B.x, B.y);
//	
//		}
//		else {
//			LED_PORT |= (1<<LED_PIN);
//		}

		
	}
	
	return 0;
	
}

// INTERRUPT FUNCTIONS

ISR(USART_RXC_vect) {
	
	//LED_PORT ^= (1<<LED_PIN);
	
	//check to see if we are ready to read a byte
	while ((UCSRA & (1 << RXC)) == 0) {}; 
	
//	rxBuffer[rxbc] = UDR;
	
	uint8_t byte = UDR;
	
//	if (byte == SERIAL_DLE && !inDelimiter) {
//		inDelimiter = 1;
//	}
//	else {
//		rxBufferPush(byte);
//		inDelimiter = 0;
//	}
	
	rxBufferPush(byte);
	
	sendUSART('*');
	
}


uint8_t rxBufferSize() {
	
	//disable interrupts
	//so that head or tail values don't change due to serial interrupt
	//cli();
	
	if (head >= tail) {
		return head - tail;
	} 
	else {
		return RX_BUFFERSIZE - (tail - head);
	}
	
	//sei();

}

uint8_t rxBufferPeek(uint8_t offset) {

	uint8_t p = (tail+offset) % RX_BUFFERSIZE;
	return rxBuffer[p];
	
}

void rxBufferDiscard(uint8_t n) {

	cli();
	
	tail+= n;
	tail%= RX_BUFFERSIZE;
	
	sei();
	
}
	

void rxBufferPush(uint8_t b) {
	
	rxBuffer[head] = b;

	head++;
	head %= RX_BUFFERSIZE;
	
	// If head meets tail we have overrun, so nudge tail and lose the oldest byte
	if (head == tail) {
		tail++;
		tail %= RX_BUFFERSIZE;
	}
	
}

uint8_t decodeNext(void) {

	uint8_t currentSize = rxBufferSize();
	
	//if less than 3 it doesn't have a start, end and middle...
	if (currentSize < 3) return 0;
	
	sendUSART('a');
	
	if (rxBufferPeek(0) != SERIAL_STX) {
		rxBufferDiscard(1);
		return 0;
	}
	
	sendUSART('b');
	
	uint8_t commandCode = rxBufferPeek(1);
	uint8_t messageLength = 0;
	
	switch (commandCode) {
		case COMMAND_CODE_PEN_DOWN:
			messageLength = MESSAGE_LENGTH_PEN_DOWN;
			break;
		case COMMAND_CODE_PEN_UP:
			messageLength = MESSAGE_LENGTH_PEN_UP;
			break;
		case COMMAND_CODE_MOVE_ABS:
			messageLength = MESSAGE_LENGTH_MOVE_ABS;
			break;
		case COMMAND_CODE_MOVE_REL:
			messageLength = MESSAGE_LENGTH_MOVE_REL;
			break;
			
		default:
			break;
	}
	
	//if couldn't classify message discard and return
	if (messageLength == 0) {
		rxBufferPeek(1);
		return 0;
	}
	
	sendUSART('c');
	
	//if the full message hasn't arrived, wait...
	if (currentSize < messageLength) {
		return 0;
	}
	
	sendUSART('d');
	
	if (rxBufferPeek(messageLength-1) != SERIAL_ETX)  {
		rxBufferDiscard(1);
		return 0;
	}
	
	sendUSART('e');
	
	currentCommand.commandCode = commandCode;
	
	switch (commandCode) {
		case COMMAND_CODE_PEN_DOWN:
			//nothing here, as pen down has no parameters
			break;
		case COMMAND_CODE_PEN_UP:
			//same here...
			break;
		case COMMAND_CODE_MOVE_ABS:
			currentCommand.command.moveAbs.x = (rxBufferPeek(2) | (rxBufferPeek(3)<<8));
			currentCommand.command.moveAbs.y = (rxBufferPeek(4) | (rxBufferPeek(5)<<8));
			break;
		case COMMAND_CODE_MOVE_REL:
			currentCommand.command.moveRel.x = (rxBufferPeek(2) | (rxBufferPeek(3)<<8));
			currentCommand.command.moveRel.y = (rxBufferPeek(4) | (rxBufferPeek(5)<<8));
			break;
			
		default:
			break;
	}
	
	int i;
	for (i = 0; i < messageLength; i++) {
		uint8_t byte = rxBufferPeek(i);
		
		uint8_t nibble;
		nibble = byte >> 4;
		if (nibble >= 10) {
			nibble+= ('A'-10);
		}
		else {
			nibble+= '0';
		}
		
		sendUSART(nibble);
		
		nibble = byte & 15;
		if (nibble >= 10) nibble+= ('A'-10);
		else nibble+= '0';
		sendUSART(nibble);
				
							
	}
	
	
	
	rxBufferDiscard(messageLength);
	sendUSART('z');
	return 1;
	
	
	
}


void moveAbs(uint16_t x, uint16_t y) {
	sendUSART('m');
	line(pos.x, pos.y, x, y);
	sendUSART('n');
}

void moveRel(int16_t x, int16_t y) {
	line(pos.x, pos.y, ((uint16_t)((int16_t)pos.x)+x), ((uint16_t)((int16_t)pos.y)+y));
}


//void popUSART(void) {
//
//	//disable interrupts
//	cli();
//	
//u	tail++;
//	
//	if (tail >= RX_BUFFERSIZE) {
//		tail = 0;
//	}
//	
//	//and now enable
//	sei();  
//}


//uint8_t peekMessageBuffer(uint8_t offset) {
//
//	uint8_t p = (tail+offset) % RX_BUFFERSIZE;
//	return rxBuffer[p];
//	
//}

//void getNext(void) {
//	
//	if (peekUSART(0) == MOVE) {
//	
//	//	A.x = (peekUSART(1) | (peekUSART(2) << 8));
//	//	A.y = (peekUSART(3) | (peekUSART(4) << 8));
//		A.x = LastB.x;
//		A.y = LastB.y;
//		B.x = (peekUSART(1) | (peekUSART(2) << 8));
//		B.y = (peekUSART(3) | (peekUSART(4) << 8));
//		LastB.x = B.x;
//		LastB.y = B.y;
//		
//		nextLineFlag = 1;
//		
//		for (int i = 0; i < RX_PACKET_SIZE; i++) {
//			popUSART();
//		}
//		
//	}
//	
//}


// MAIN FUNCTIONS

/* - - - LINE - - -
 
 thanks to RogueBasin for the meat of this Bresenham line algorithm
 http://roguebasin.roguelikedevelopment.org/index.php?title=Bresenham%27s_Line_Algorithm
 
 */

void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	
	int8_t xstep = 1;
	int8_t ystep = 1;
	
	if (x0 > x1) xstep = -1;
	if (y0 > y1) ystep = -1;
	
	int16_t dx = abs(x1 - x0);
	int16_t dy = abs(y1 - y0);
	
	int16_t x = x0;
	int16_t y = y0;
		
	wakeMotors();
	
	//a boolean for sending for next coordiantes
	uint8_t send = 0;
		
	if (dy <= dx) {
		int16_t error = dy - dx;
		
		while (x != x1) {
			if (error >= 0) {
				if (error || (xstep > 0)) {
					moveHalfStep(2, stepCount2);
					stepCount2+= ystep;
					y+= xstep;
					error-= dx;
				}
			}
			
			moveHalfStep(1, stepCount1);
			stepCount1+= xstep;
			
			x+= xstep;
			error+= dy;
			
			//delay once...
			delay_ms(delayTime);
			
			//send for new coordinates
			//do this once, halfway through the line
			if (abs(x) > (dx/2) && !send) {
				//sendUSART(nextPos);
				//getNext();
				send = 1;
			}
			
			//update pos
			pos.x = x;
			pos.y = y;
			
		}// end while
	}
	
	//if dy > dx
	else {
		int16_t error = dx - dy;
		
		while (y != y1) {
			
			if (error >= 0) {
				if (error || (ystep > 0)) {
					moveHalfStep(1, stepCount1);
					stepCount1+= xstep;
					x+= xstep;
					error-= dy;
				}
			}
			
			moveHalfStep(2, stepCount2);
			stepCount2+= ystep;
			
			y+= ystep;
			error+= dx;
			
			//delay once...
			delay_ms(delayTime);
			
			//send for new coordinates
			//do this once, halfway through the line
			if (abs(y) > (dy/2) && !send) {
				//sendUSART(nextPos);
				send = 1;
			}
			
			//update pos
			pos.x = x;
			pos.y = y;
			
		}// end while
	}
	
	
	//this is used by single lines
//	if (nextLineFlag == 0) {
//		sendUSART(done);
//	}
	
	//check to see if we have reached destination...
	//by this point B would have changed if there was a next coordiante
	//as it is fetched in the middle of the line
	if (x1 == B.x && y1 == B.y) {
		nextLineFlag = 0;
		sendUSART(done);
	}
	
	sleepMotors();
	
	//if roughly halfway through 6 points
//	if (pos.x == P[2].x && pos.y == P[2].y) {
//		sendUSART(34);
//	}
	
}


void processUSART(void) {
	
	switch (rxBuffer[0]) {
			
		//change delay time
		case 0:
			delayTime = rxBuffer[1];
			sendUSART(done);
			break;
			
		//single line
		case 1:
			nextLineFlag = 0;
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
		
		//single move
		case 3:
			nextLineFlag = 0;
			line(pos.x, pos.y, (rxBuffer[1] | (rxBuffer[2]<<8)), (rxBuffer[3] | (rxBuffer[4]<<8)));
			break;

		//multiple move
		case MOVE:
			nextLineFlag = 1;
			A.x = LastB.x;
			A.y = LastB.y;
			B.x = (rxBuffer[1] | (rxBuffer[2]<<8));
			B.y = (rxBuffer[3] | (rxBuffer[4]<<8));
			
			if (B.x == pos.x && B.y == pos.y) {
				sendUSART(nextPos);
			}
			
			break;
			
		//get pos, send pos coordinates
		case 5:
			sendUSART(pos.x);
			sendUSART(pos.x>>8);
			sendUSART(pos.y);
			sendUSART(pos.y>>8);
			
			sendUSART(done);
			break;
			
		//move pen up
		case 6:
			//penUp();
			sendUSART(done);
			break;

		//move pen down
		case 7:
			//penDown();
			sendUSART(done);
			break;
			
	}// end switch
	
	
}

void sendUSART(uint8_t byte) {
	while ((UCSRA & (1 << UDRE)) == 0) {};
	UDR = byte;
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
void moveHalfStep(char motor, uint8_t stepCount) {
	
	
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


void delay_ms(int d) {	
	while (d) {
		_delay_ms(0.25);
		d--;
	}
}

// PEN FUNCTIONS

void movePenUp(void) {
	PEN_PORT &= ~(1<<PEN_PIN);
}

void movePenDown(void) {
	PEN_PORT |= (1<<PEN_PIN);
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
	
	sleepMotors();
	
	delayTime = 16;
}

void initPen(void) {
	
	PEN_DDR |= (1<<PEN_PIN);
	//start with pen up
	//movePenUp();
}	

void initTimers(void) {
	
	//set timer0 to prescale of 1024
	TCCR0 |= (1<<CS00) | (1<<CS02);
	
	TCCR1B |= (1<<CS10) | (1<<CS12);
	
	//set timer2 to prescale of 1024
	TCCR2 |= (1<<CS22) | (1<<CS21) | (1<<CS20);
}

