
#ifndef _USART
#define _USART

#include "main.h"
#include "commands.h"
#include <stdint.h>

#define RX_BUFFER_STX_ERROR 87
#define RX_BUFFER_LENGTH_ERROR 91
#define RX_BUFFER_ETX_ERROR 102

#define COMMAND_BUFFER_N0_STX_ERROR 32
#define COMMAND_BUFFER_LENGTH_ERROR 6

#define SEND_FOR_NEXT_COMMANDS 213
#define PLOTTER_FINISHED_DRAWING 182

#define RX_BUFFER_SIZE 180
#define COMMAND_BUFFER_SIZE 300

#define COMMAND_BUFFER_FULL 57
#define RECEIVED_RX_PACKET 6
#define RX_SEND_NEXT 7

volatile uint8_t rxBuffer[RX_BUFFER_SIZE]; //this holds all the serial data
volatile uint8_t rHead, rTail;

volatile uint8_t rxDelayedCounter;
volatile uint8_t commandBuffer[COMMAND_BUFFER_SIZE];
volatile uint8_t cHead, cTail;
volatile uint8_t noCommands;
volatile uint8_t rxDelayedIndex;
uint8_t doDelayedBuffer;

void commandBufferPush(uint8_t byte);
uint8_t commandBufferPop();
void commandBufferReset(void);

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