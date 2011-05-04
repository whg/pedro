
#include "usart.h"

uint8_t rxBufferSize() {
	
	if (rHead >= rTail) {
		return rHead - rTail;
	} 
	else {
		return RX_BUFFER_SIZE - (rTail - rHead);
	}
}

uint8_t rxBufferPeek(uint8_t offset) {
	
	uint8_t p = (rTail+offset) % RX_BUFFER_SIZE;
	return rxBuffer[p];
	
}

void rxBufferDiscard(uint8_t n) {
	
	uint8_t sreg = SREG;
	cli(); 
	
	rTail+= n;
	rTail%= RX_BUFFER_SIZE;
	
	SREG = sreg;
}


void rxBufferPush(uint8_t b) {
	
	uint8_t next = (rHead+1) % RX_BUFFER_SIZE;
	
	if (rTail == next) {
		//throw away byte, buffer full
		return; 
	}
	
	rxBuffer[rHead] = b;
	rHead = next;
	
}

void commandBufferPush(uint8_t byte) {
	
	uint8_t next = (cHead+1) % COMMAND_BUFFER_SIZE;
	
	if (cTail == next) {
		sendUSART(COMMAND_BUFFER_FULL);
	}
	else {
		commandBuffer[cHead] = byte;
		cHead = next;
	}
	
}

uint8_t commandBufferPop() {

	uint8_t byte = commandBuffer[cTail];
	
	cTail++; 
	cTail %= COMMAND_BUFFER_SIZE;
	
	return byte;
	
}

void commandBufferReset() {
	cHead = 0;
	cTail = 0;
}

uint8_t decodeNext(void) {
	
	
	uint8_t currentSize = rxBufferSize();
	
	//if less than 3 it doesn't have a start, end and middle...
	if (currentSize < 3) return 0;
	
	if (rxBufferPeek(0) != SERIAL_STX) {
		sendUSART(RX_BUFFER_STX_ERROR);
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
		case COMMAND_CODE_EXECUTE_COMMANDS:
			messageLength = MESSAGE_LENGTH_EXECUTE_COMMANDS;
			break;
		case COMMAND_CODE_CHANGE_STEP_DELAY:
			messageLength = MESSAGE_LENGTH_CHANGE_STEP_DELAY;
			break;
		case COMMAND_CODE_LAST_LOT:
			messageLength = MESSAGE_LENGTH_LAST_LOT;
			break;
		case COMMAND_CODE_FINISH:
			messageLength = MESSAGE_LENGTH_FINISH;
			break;
		case COMMAND_CODE_NUM_COMMANDS:
			messageLength = MESSAGE_LENGTH_NUM_COMMANDS;
			break;

		default:
			break;
	}
	
	//if couldn't classify message discard and return
	if (messageLength == 0) {
		sendUSART(RX_BUFFER_LENGTH_ERROR);
		rxBufferDiscard(1);
		return 0;
	}
	
	//if the full message hasn't arrived, wait...
	if (currentSize < messageLength) {
		return 0;
	}
	
	if (rxBufferPeek(messageLength-1) != SERIAL_ETX)  {
		sendUSART(RX_BUFFER_ETX_ERROR);
		rxBufferDiscard(1);
		return 0;
	}
		
	switch (commandCode) {
		case COMMAND_CODE_GET_POS:
			sendPos();
			rxBufferDiscard(messageLength);
			return 0;
			
		case COMMAND_CODE_QUERY_DELAYED:
			sendUSART(noCommands);
			rxBufferDiscard(messageLength);
			return 0;
			
//		case COMMAND_CODE_EXECUTE_COMMANDS:
//			doCommands = 1;
//			rxBufferDiscard(messageLength);
//			return 1;
			
		case COMMAND_CODE_LAST_LOT:
			lastLot = rxBufferPeek(2);
			rxBufferDiscard(messageLength);
			return 0;

		case COMMAND_CODE_NUM_COMMANDS:
			numCommands = (rxBufferPeek(2) | (rxBufferPeek(3)<<8));
			rxBufferDiscard(messageLength);
			return 0;
			
		case COMMAND_CODE_FINISH:
			sendUSART(PLOTTER_FINISHED_DRAWING);
			rxBufferDiscard(messageLength);
			return 0;


		default:
			break;
	}
	
	//if we get here...
	//...add all the bytes to the command buffer
	//so that they get processed when we start...
	int i;
	for (i = 0; i < messageLength; i++) {
		commandBufferPush(rxBufferPeek(i));
	}
	
	//one more command
	//noCommands++;
	
//	sendUSART(99);
//	sendUSART(messageLength);
//	sendUSART(noCommands);
	
//	delay_ms(1);
	
	rxBufferDiscard(messageLength);
	return 1;
	
}

//void addToDelayedBuffer(uint8_t byte) {
//	
//	rxDelayedBuffer[rxDelayedCounter] = byte;
//	rxDelayedCounter++;
//	
//	if (rxDelayedCounter >= RX_BUFFER_SIZE - 1) {
//		sendUSART(RX_BUFFER_FULL);
//	}
//	
//	
//}
//
//uint8_t delayedBufferPeek(uint8_t index, uint8_t offset) {
//	return rxDelayedBuffer[index+offset];
//}


/*
 - - - SENDING - - -
 */

void sendUSART(uint8_t byte) {
	while ((UCSRA & (1 << UDRE)) == 0) {};
	UDR = byte;
}

void sendUSART16(uint16_t thing) {
	
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