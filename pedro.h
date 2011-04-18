
#include <avr/io.h>
#define F_CPU 14745600UL
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define BAUDRATE 9600
//baud prescale will be 95 for 14.7456MHz and 9600 baud rate
#define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 16UL))) - 1) 

#define NO_STEPS 8
#define DELAY 0.25;

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

#define RX_BUFFERSIZE 9

#define LED_PORT PORTB
#define LED_PIN PORTB0
#define LED_DDR DDRB

// STRUCTS
typedef struct {
	uint8_t dir;
	uint16_t noSteps;
	uint16_t timeDelay;
	uint16_t counter;
} Motor;

typedef struct {
	uint16_t x;
	uint16_t y;
} Point;


// INIT FUNCTIONS
void initUSART(void);
void initMotors(void);
void initTimers(void);


// MAIN FUNCTIONS
void changeStep(char motor, char p1, char p2, char p3, char p4);
void delay_ms(int d);
void moveHalfStep(char motor, int8_t stepCount);
void setMotors(void);
void processUSART(void);
void sleepMotors(void); //don't need to have two of these
void wakeMotors(void);  //but i think it makes things clearer

void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// UTIL FUNCTIONS
void swap16(uint16_t *a, uint16_t *b);

// DATA
Point pos;
volatile Point A, B;
int8_t stepCount1, stepCount2;

volatile uint8_t rxBuffer[RX_BUFFERSIZE]; //this holds all the serial data
volatile uint8_t rxbc; // rx buffer counter

volatile uint8_t instructions[RX_BUFFERSIZE];

volatile uint8_t delayTime;
volatile uint8_t nextLineFlag;
