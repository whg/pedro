PRG            = pedro
OBJ            = main.o line.o motors.o usart.o
PROGRAMMER     = usbtiny
MCU_TARGET     = atmega8 
AVRDUDE_TARGET = m8
OPTIMIZE       = -Os 
CC             = avr-gcc
 
# Override is only needed by avr-lib build system.
 
override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map
 
OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
 
all: $(PRG).elf lst text #eeprom
 
$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
 
clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak *.hex *.bin *.srec
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
 
lst:  $(PRG).lst
 
%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@
 
# Rules for building the .text rom images
 
text: hex
 
hex:  $(PRG).hex
 
%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	avr-size $(PRG).hex
 
 
# command to program chip (invoked by running "make install")
install:  $(PRG).hex
	avrdude -p $(AVRDUDE_TARGET) -c $(PROGRAMMER) -U flash:w:$(PRG).hex 
 
fuse:
	avrdude -p $(AVRDUDE_TARGET) -c $(PROGRAMMER) \
	-U lfuse:w:0xc6:m -U hfuse:w:0xd9:m 	
 