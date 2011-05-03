
#include "usart.h"

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