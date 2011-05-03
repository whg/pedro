
#include "main.h"

int main (void) { 
	
	LED_DDR |= (1<<LED_PIN);
	LED_PORT &= ~(1<<LED_PIN);
	
	initUSART();
	initMotors();
	initPen();
	initTimers();
	
	rHead = 1;
	rTail = 0;
	
	cHead = 0;
	cTail = 0;
	
	sei(); 
	while (1) {
		
		decodeNext();
		
		if (doCommands) {
			
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


uint8_t executeDelayedBuffer(void) {
	
	if (commandBufferPop() != SERIAL_STX) {
		sendUSART(COMMAND_BUFFER_N0_STX_ERROR); 
		return 0; 
	}
	
	uint8_t commandCode = commandBufferPop();
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
		case COMMAND_CODE_FINISH:
			messageLength = MESSAGE_LENGTH_FINISH;
			break;

			
		default:
			break;
	}
	
	//if couldn't classify message discard and return
	if (messageLength == 0) {
		sendUSART(COMMAND_BUFFER_LENGTH_ERROR);
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
			currentCommand.command.moveAbs.x = (commandBufferPop() | (commandBufferPop()<<8));
			currentCommand.command.moveAbs.y = (commandBufferPop() | (commandBufferPop()<<8));
			break;
		case COMMAND_CODE_MOVE_REL:
			currentCommand.command.moveRel.x = (commandBufferPop() | (commandBufferPop()<<8));
			currentCommand.command.moveRel.y = (commandBufferPop() | (commandBufferPop()<<8));
			break;
		case COMMAND_CODE_GET_POS:
			//no parameters here...
			break;
		case COMMAND_CODE_CHANGE_STEP_DELAY:
			currentCommand.command.changeDelay.time = commandBufferPop();
			break;
			
		case COMMAND_CODE_FINISH:
			doCommands = 0;
			noCommands = 0;
			break;

		default:
			break;
	}
	
	
	if ((commandCounter == 20) && !lastLot) {
		//stop all motor action
		doCommands = 0;
		
		//reset counter
		commandCounter = 0;
		
		//send for next lot
		sendUSART(SEND_FOR_NEXT_COMMANDS);
		
		commandBufferReset();
		return 0;
	}
	
	commandCounter++;

	//pop off the DTX
	commandBufferPop();
	
	
//	//rxBufferDiscard(messageLength);
//	rxDelayedIndex+= messageLength;
//	
//	if (rxDelayedIndex >= rxDelayedCounter) {
//		doDelayedBuffer = 0;
//		rxDelayedIndex = 0;
//		rxDelayedCounter = 0;
//		rxDelayedNoCommands = 0;
//		sendUSART(RX_SEND_NEXT);
//	}
	return 1;
	
	
}

//MAIN FUNCTIONS

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

void delay_ms(int d) {	
	while (d) {
		_delay_ms(DELAY);
		d--;
	}
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
