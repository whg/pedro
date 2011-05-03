 

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
	
	head = 1;
	tail = 0;
	
	sei();

	while (1) {
		
		decodeNext();
		
		if (doDelayedBuffer) {

			if (executeDelayedBuffer()) { 
				//got next command to process
	
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
					case COMMAND_CODE_CHANGE_STEP_DELAY:
						changeStepDelay(currentCommand.command.changeDelay.time);
						break;

						
					default:
						break;
				}
				

			}
		}
				
		else {
			//no commands to process
			
			
			//if idle for a while turn motors off...
		}
 
		
	}
	
	return 0;
	
}

// INTERRUPT FUNCTIONS

ISR(USART_RXC_vect) {
	
	uint8_t byte = UDR;
	rxBufferPush(byte);
	
}


uint8_t rxBufferSize() {

	if (head >= tail) {
		return head - tail;
	} 
	else {
		return RX_BUFFERSIZE - (tail - head);
	}
	
}

uint8_t rxBufferPeek(uint8_t offset) {

	uint8_t p = (tail+offset) % RX_BUFFERSIZE;
	return rxBuffer[p];
	
}

void rxBufferDiscard(uint8_t n) {

	uint8_t sreg = SREG;
	cli();
	
	tail+= n;
	tail%= RX_BUFFERSIZE;
	
	SREG = sreg;
}
	

void rxBufferPush(uint8_t b) {
	
	uint8_t next = (head+1)%RX_BUFFERSIZE;

	if (tail == next) {
		//throw away byte, buffer full
		return;
	}
	
	rxBuffer[head] = b;
	head = next;

}


uint8_t decodeNext(void) {
	

	uint8_t currentSize = rxBufferSize();
	
	//if less than 3 it doesn't have a start, end and middle...
	if (currentSize < 3) return 0;
		
	if (rxBufferPeek(0) != SERIAL_STX) {
		rxBufferDiscard(1);
		return 0; 
	}
		
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
		case COMMAND_CODE_GET_POS:
			messageLength = MESSAGE_LENGTH_GET_POS;
			break;
		case COMMAND_CODE_QUERY_DELAYED:
			messageLength = MESSAGE_LENGTH_QUERY_DELAYED;
			break;
		case COMMAND_CODE_EXECUTE_DELAYED:
			messageLength = MESSAGE_LENGTH_EXECUTE_DELAYED;
			break;
		case COMMAND_CODE_CHANGE_STEP_DELAY:
			messageLength = MESSAGE_LENGTH_CHANGE_STEP_DELAY;
			break;

		default:
			break;
	}
		
	//if couldn't classify message discard and return
	if (messageLength == 0) {
		rxBufferDiscard(1);
		return 0;
	}
		
	//if the full message hasn't arrived, wait...
	if (currentSize < messageLength) {
		return 0;
	}
		
	if (rxBufferPeek(messageLength-1) != SERIAL_ETX)  {
		rxBufferDiscard(1);
		return 0;
	}
		
	currentCommand.commandCode = commandCode;
	
	switch (commandCode) {
		case COMMAND_CODE_GET_POS:
			sendPos();
			rxBufferDiscard(messageLength);
			return 1;
			
		case COMMAND_CODE_QUERY_DELAYED:
			sendUSART(rxDelayedNoCommands);
			rxBufferDiscard(messageLength);
			return 1;
			break;

		case COMMAND_CODE_EXECUTE_DELAYED:
			doDelayedBuffer = 1;
			rxBufferDiscard(messageLength);
			return 1;

		default:
			break;
	}
	
	//if we get here...
	//...add all the bytes to the delayed buffer
	//so that they get processed when we start...
	int i;
	for (i = 0; i < messageLength; i++) {
		addToDelayedBuffer(rxBufferPeek(i));
	}
	
	//one more command
	rxDelayedNoCommands++;
	 	
	rxBufferDiscard(messageLength);
	return 1;

}

void addToDelayedBuffer(uint8_t byte) {
	
	rxDelayedBuffer[rxDelayedCounter] = byte;
	rxDelayedCounter++;
	
	if (rxDelayedCounter >= RX_BUFFERSIZE - 1) {
		sendUSART(RX_BUFFER_FULL);
	}

	
}

uint8_t delayedBufferPeek(uint8_t index, uint8_t offset) {
	return rxDelayedBuffer[index+offset];
}

uint8_t executeDelayedBuffer(void) {

	if (delayedBufferPeek(rxDelayedIndex, 0) != SERIAL_STX) {
		rxDelayedIndex++;
		return 0; 
	}
	
	uint8_t commandCode = delayedBufferPeek(rxDelayedIndex, 1);
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
		case COMMAND_CODE_GET_POS:
			messageLength = MESSAGE_LENGTH_GET_POS;
			break;
		case COMMAND_CODE_CHANGE_STEP_DELAY:
			messageLength = MESSAGE_LENGTH_CHANGE_STEP_DELAY;
			break;

		default:
			break;
	}
	
	//if couldn't classify message discard and return
	if (messageLength == 0) {
		rxDelayedIndex++;
//		sendUSART('e');
		return 0;
	}
	
	if (delayedBufferPeek(rxDelayedIndex, (messageLength-1)) != SERIAL_ETX)  {
		rxDelayedIndex++;
		//sendUSART('f');
		return 0;
	}
	
	currentCommand.commandCode = commandCode;
	
	switch (commandCode) {
		case COMMAND_CODE_PEN_DOWN:
			//nothing here, as pen down has no parameters
			break;
		case COMMAND_CODE_PEN_UP:
			//same here...
			break;
		case COMMAND_CODE_MOVE_ABS:
			currentCommand.command.moveAbs.x = (delayedBufferPeek(rxDelayedIndex, 2) | (delayedBufferPeek(rxDelayedIndex, 3)<<8));
			currentCommand.command.moveAbs.y = (delayedBufferPeek(rxDelayedIndex, 4) | (delayedBufferPeek(rxDelayedIndex, 5)<<8));
			break;
		case COMMAND_CODE_MOVE_REL:
			currentCommand.command.moveRel.x = (delayedBufferPeek(rxDelayedIndex, 2) | (delayedBufferPeek(rxDelayedIndex, 3)<<8));
			currentCommand.command.moveRel.y = (delayedBufferPeek(rxDelayedIndex, 2) | (delayedBufferPeek(rxDelayedIndex, 3)<<8));
			break;
		case COMMAND_CODE_GET_POS:
			//no parameters here...
			break;
		case COMMAND_CODE_CHANGE_STEP_DELAY:
			currentCommand.command.changeDelay.time = delayedBufferPeek(rxDelayedIndex, 2);
			break;

			
		default:
			break;
	}
	
	//rxBufferDiscard(messageLength);
	rxDelayedIndex+= messageLength;
	
	if (rxDelayedIndex >= rxDelayedCounter) {
		doDelayedBuffer = 0;
		rxDelayedIndex = 0;
		rxDelayedCounter = 0;
		rxDelayedNoCommands = 0;
		sendUSART(RX_SEND_NEXT);
	}
	return 1;
	
	
}


void moveAbs(uint16_t x, uint16_t y) {
	lineTo(x, y);
}

void moveRel(int16_t x, int16_t y) {
	//line(pos.x, pos.y, ((uint16_t)((int16_t)pos.x)+x), ((uint16_t)((int16_t)pos.y)+y));
}

void sendPos(void) {
	
	sendUSART(pos.x);
	sendUSART(pos.x>>8);
	sendUSART(pos.y);
	sendUSART(pos.y>>8);
	
}



// MAIN FUNCTIONS

/* - - - LINE - - -
 
 thanks to RogueBasin for the meat of this Bresenham line algorithm
 http://roguebasin.roguelikedevelopment.org/index.php?title=Bresenham%27s_Line_Algorithm
 it needed a little touching up though...
 
 */

void lineTo(uint16_t x1, uint16_t y1) {
	
	uint16_t x0 = pos.x;
	uint16_t y0 = pos.y;
	
//	send16(pos.x);
//	send16(pos.y);
//	send16(x1);
//	send16(y1);
	
	// - - -
	
	int8_t xstep = 1;
	int8_t ystep = 1;
	
	if (x0 > x1) xstep = -1;
	if (y0 > y1) ystep = -1;
	
	int16_t dx = abs(x1 - x0);
	int16_t dy = abs(y1 - y0);
	
	int16_t x = x0;
	int16_t y = y0;
		
	wakeMotors();

	if (dy <= dx) {
		int16_t error = dy - dx;
		
		while (x != x1) {
			if (error >= 0) {
				if (error || (xstep > 0)) {
					moveHalfStep(2);
					stepCount2+= ystep;
					y+= ystep;
					error-= dx;
				}
			}
			
			moveHalfStep(1);
			stepCount1+= xstep;
			
			x+= xstep;
			error+= dy;
			
			//delay once...
			delay_ms(delayTime);
			
			//update pos
			pos.x = x;
			pos.y = y;
			
		}// end while
		
		//a little readjustment, as algorithm isn't quite right...
		if (x0 > x1) {
			y+= ystep;
			pos.y = y;
		}		
	}
	
	//if dy > dx
	else {
		int16_t error = dx - dy;
		
		while (y != y1) {
			
			if (error >= 0) {
				if (error || (ystep > 0)) {
					moveHalfStep(1);
					stepCount1+= xstep;
					x+= xstep;
					error-= dy;
				}
			}
			
			moveHalfStep(2);
			stepCount2+= ystep;
			
			y+= ystep;
			error+= dx;
			
			//delay once...
			delay_ms(delayTime);
			
			
			//update pos
			pos.x = x;
			pos.y = y;
			
		}// end while
		
		if (y1 < y0) {
			x+= xstep;
			pos.x = x;
		}
	}


	sleepMotors();
	
//	send16(pos.x);
//	send16(pos.y);
	
	pos.x = x1;
	pos.y = y1;
	
}

void send16(uint16_t thing) {
	
	sendUSART('-');

	uint8_t byte = (thing>>8);
	
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
	
	byte = thing;
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


void sendUSART(uint8_t byte) {
	while ((UCSRA & (1 << UDRE)) == 0) {};
	UDR = byte;
}

void sleepMotors(void) {
	M1_PORT &= ~(1<<M1_ENABLE);
	M2_PORT &= ~(1<<M2_ENABLE);
}

void wakeMotors(void) {
	M1_PORT |= (1<<M1_ENABLE);
	M2_PORT |= (1<<M2_ENABLE);
}


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


void delay_ms(int d) {	
	while (d) {
		_delay_ms(0.25);
		d--;
	}
}

// PEN FUNCTIONS

void movePenUp(void) {
	PEN_PORT &= ~(1<<PEN_PIN);
	LED_PORT &= ~(1<<LED_PIN);
}

void movePenDown(void) {
	PEN_PORT |= (1<<PEN_PIN);
	LED_PORT |= (1<<LED_PIN);
}

void changeStepDelay(uint8_t ms) {
	delayTime = ms;
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
	
	wakeMotors();
	
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

