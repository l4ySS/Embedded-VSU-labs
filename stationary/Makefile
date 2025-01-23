CC = avr-gcc
OBJCOPY = avr-objcopy
MMCU = atmega88
F_CPU = 16000000UL
TARGET = main
SRC = main.c config.c
LIB_DIR = lib
LIB_SRC = $(wildcard $(LIB_DIR)/*.c)
OBJ = $(SRC:.c=.o) $(LIB_SRC:.c=.o)
DEPS = $(wildcard $(LIB_DIR)/*.h)

CFLAGS = -Wall -Wextra -mmcu=$(MMCU) -DF_CPU=$(F_CPU) -Os -I$(LIB_DIR)
LDFLAGS = -mmcu=$(MMCU)

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET).elf: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET).elf $(TARGET).hex