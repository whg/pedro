#name
FILE=pedro
#microprocessor type
MCU=atmega8
#
CC=avr-gcc
OBJCOPY=avr-objcopy
# optimize for size:
CFLAGS=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os -mcall-prologues
#go for it
$(FILE).hex : $(FILE).out 
	avr-size $(FILE).out
	$(OBJCOPY) -R .eeprom -O ihex $(FILE).out $(FILE).hex 
$(FILE).out : $(FILE).o 
	$(CC) $(CFLAGS) -o $(FILE).out -Wl,-Map,$(FILE).map $(FILE).o 
$(FILE).o : $(FILE).c 
	$(CC) $(CFLAGS) -Os -c $(FILE).c