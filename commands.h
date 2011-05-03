
#ifndef _COMMANDS
#define _COMMANDS

#include <stdint.h>


#define COMMAND_CODE_PEN_DOWN 0
#define COMMAND_CODE_PEN_UP 1
#define COMMAND_CODE_MOVE_ABS 2
#define COMMAND_CODE_MOVE_REL 3
#define COMMAND_CODE_GET_POS 4
#define COMMAND_CODE_QUERY_DELAYED 5
#define COMMAND_CODE_EXECUTE_DELAYED 6
#define COMMAND_CODE_CHANGE_STEP_DELAY 7

#define MESSAGE_LENGTH_PEN_DOWN 3
#define MESSAGE_LENGTH_PEN_UP 3
#define MESSAGE_LENGTH_MOVE_ABS 7
#define MESSAGE_LENGTH_MOVE_REL 7
#define MESSAGE_LENGTH_GET_POS 3
#define MESSAGE_LENGTH_QUERY_DELAYED 3
#define MESSAGE_LENGTH_EXECUTE_DELAYED 3
#define MESSAGE_LENGTH_CHANGE_STEP_DELAY 4

//all structs are typedef'ed so they can all go in a union
//this union is then part of a command descriptor

typedef struct {
	
} COMMAND_PEN_DOWN;


typedef struct {
	
} COMMAND_PEN_UP;

typedef struct {
	uint16_t x;
	uint16_t y;
} COMMAND_MOVE_ABS;


typedef struct {
	int16_t x;
	int16_t y;
} COMMAND_MOVE_REL;


typedef struct {
} COMMAND_GET_POS;


typedef struct {
} COMMAND_QUERY_DELAYED;


typedef struct {
} COMMAND_EXECUTE_DELAYED;


typedef struct {
	uint8_t time;
} COMMAND_CHANGE_DELAY;

typedef union {
	COMMAND_PEN_DOWN penDown;
	COMMAND_PEN_UP penUp;
	COMMAND_MOVE_ABS moveAbs;
	COMMAND_MOVE_REL moveRel;	
	COMMAND_CHANGE_DELAY changeDelay;
} COMMAND;

typedef struct {
	uint8_t commandCode;
	COMMAND command;
} COMMAND_DESCRIPTOR;


#endif