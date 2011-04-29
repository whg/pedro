
#include <avr/io.h>
#define F_CPU 14745600UL
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define BAUDRATE 28800
//baud prescale will be 95 for 14.7456MHz and 9600 baud rate
//and 31 for 28.8k
#define BAUD_PRESCALE 31

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

#define PEN_PORT PORTD
#define PEN_DDR DDRD
#define PEN_PIN PORTD2

#define RX_BUFFERSIZE 90

#define LED_PORT PORTB
#define LED_PIN PORTB0
#define LED_DDR DDRB

#define RX_PACKET_SIZE 9
#define RX_BUFFER_FULL 5
#define RECEIVED_RX_PACKET 6


#define MOVE 4

// Serial Message Format
// STX <Command> <PayLoad> <CRC> ETX
// STX=02, ETX=03, DLE=10
#define SERIAL_DLE 0x10
#define SERIAL_STX 0x02
#define SERIAL_ETX 0x03

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


#define COMMAND_CODE_PEN_DOWN 0
#define MESSAGE_LENGTH_PEN_DOWN 3

typedef struct {
	
} COMMAND_PEN_DOWN;

#define COMMAND_CODE_PEN_UP 1
#define MESSAGE_LENGTH_PEN_UP 3

typedef struct {

} COMMAND_PEN_UP;

#define COMMAND_CODE_MOVE_ABS 2
#define MESSAGE_LENGTH_MOVE_ABS 7

typedef struct {
	uint16_t x;
	uint16_t y;
} COMMAND_MOVE_ABS;

#define COMMAND_CODE_MOVE_REL 3
#define MESSAGE_LENGTH_MOVE_REL 7

typedef struct {
	int16_t x;
	int16_t y;
} COMMAND_MOVE_REL;

typedef union {
	COMMAND_PEN_DOWN penDown;
	COMMAND_PEN_UP penUp;
	COMMAND_MOVE_ABS moveAbs;
	COMMAND_MOVE_REL moveRel;	
} COMMAND;
	
typedef struct {
	uint8_t commandCode;
	COMMAND command;
} COMMAND_DESCRIPTOR;


uint8_t rxBufferSize(void);
uint8_t rxBufferPeek(uint8_t offset);
void rxBufferDiscard(uint8_t n);
void rxBufferPush(uint8_t b);

uint8_t decodeNext(void);

COMMAND_DESCRIPTOR currentCommand;

void moveAbs(uint16_t x, uint16_t y);
void moveRel(int16_t x, int16_t y);

// - - -


const uint8_t nextPos = 56;
const uint8_t done = 64;


// INIT FUNCTIONS
void initUSART(void);
void initMotors(void);
void initTimers(void);
void initPen(void);


// MAIN FUNCTIONS
void changeStep(char motor, char p1, char p2, char p3, char p4);
void delay_ms(int d);
void moveHalfStep(char motor, uint8_t stepCount);
void setMotors(void);
void processUSART(void);
void sleepMotors(void); //don't need to have two of these
void wakeMotors(void);  //but i think it makes things clearer

void movePenUp(void);
void movePenDown(void);

void sendUSART(uint8_t byte);

void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// UTIL FUNCTIONS
void swap16(uint16_t *a, uint16_t *b);

// DATA
volatile Point pos;
volatile Point A, B;
volatile uint8_t onP;
volatile Point LastB;
uint8_t stepCount1, stepCount2;
uint8_t	count;

volatile uint8_t rxBuffer[RX_BUFFERSIZE]; //this holds all the serial data
volatile uint8_t head, tail;
volatile uint8_t rxbc; // rx buffer counter

volatile uint8_t instructions[RX_BUFFERSIZE];

volatile uint8_t delayTime;
volatile uint8_t nextLineFlag;
volatile uint8_t mmmb;


volatile uint8_t inDelimiter;

//void pushMessageBuffer(uint8_t byte);
//uint8_t peekMessageBuffer(uint8_t offset);
