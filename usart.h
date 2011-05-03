
#ifndef _USART
#define _USART

#include "main.h"
#include "commands.h"
#include <stdint.h>

#define RX_BUFFERSIZE 180

#define RX_BUFFER_FULL 5
#define RECEIVED_RX_PACKET 6
#define RX_SEND_NEXT 7

volatile uint8_t rxDelayedCounter;
volatile uint8_t rxDelayedBuffer[RX_BUFFERSIZE];
volatile uint8_t rxDelayedNoCommands;
volatile uint8_t rxDelayedIndex;
uint8_t doDelayedBuffer;

volatile uint8_t rxBuffer[RX_BUFFERSIZE]; //this holds all the serial data
volatile uint8_t head, tail;
volatile uint8_t rxbc; // rx buffer counter

uint8_t rxBufferSize(void);
uint8_t rxBufferPeek(uint8_t offset);
void rxBufferDiscard(uint8_t n);
void rxBufferPush(uint8_t b);

uint8_t decodeNext(void);

void addToDelayedBuffer(uint8_t byte);
uint8_t delayedBufferPeek(uint8_t index, uint8_t offset);

void sendUSART(uint8_t byte);
void sendUSART16(uint16_t thing);



#endif