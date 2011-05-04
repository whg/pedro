
#ifndef _MAIN
#define _MAIN

#define F_CPU 14745600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "line.h"
#include "commands.h"
#include "motors.h"
#include "usart.h"

#define BAUDRATE 28800
//baud prescale will be 95 for 14.7456MHz and 9600 baud rate
//and 31 for 28.8k
#define BAUD_PRESCALE 31

#define NO_STEPS 8
#define DELAY 0.25

#define M1_PORT PORTC
#define M1_DDR	DDRC
#define M1_PIN1 PORTC0
#define M1_PIN2 PORTC1
#define M1_PIN3 PORTC2
#define M1_PIN4 PORTC3
#define M1_ENABLE PORTC4

#define M2_PORT PORTD
#define M2_DDR	DDRD
#define M2_PIN1 PORTD4
#define M2_PIN2 PORTD5
#define M2_PIN3 PORTD6
#define M2_PIN4 PORTD7
#define M2_ENABLE PORTD3

#define PEN_PORT PORTD
#define PEN_DDR DDRD
#define PEN_PIN PORTD2

#define LED_PORT PORTB
#define LED_PIN PORTB0
#define LED_DDR DDRB

// Serial Message Format: STX <Command> <PayLoad> ETX
#define SERIAL_STX 0x02
#define SERIAL_ETX 0x03


//this is what holds the command... it is defined in commands.h
COMMAND_DESCRIPTOR currentCommand;

uint8_t executeDelayedBuffer(void);

// INIT FUNCTIONS
void initUSART(void);
void initMotors(void);
void initTimers(void);
void initPen(void);


// MAIN FUNCTIONS
void moveAbs(uint16_t x, uint16_t y);
void moveRel(int16_t x, int16_t y);
void sendPos(void);
void movePenUp(void);
void movePenDown(void);
void changeStepDelay(uint8_t ms);

void housekeep(void);

uint8_t doCommands;
uint16_t	commandCounter;
uint8_t lastLot;
uint16_t numCommands;

// DATA
typedef struct {
	uint16_t x;
	uint16_t y;
} Point;

volatile Point pos;
volatile uint8_t delayTime;

//GENERIC
void delay_ms(int d);


#endif
