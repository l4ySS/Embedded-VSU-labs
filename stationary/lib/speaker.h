#ifndef LIB_H
#define LIB_H
#define __DELAY_BACKWARD_COMPATIBLE__
#include "../config.c"
#include "stdio.h"
#include "avr/io.h"
#include "util/delay.h"

void playNote(uint16_t frequency, uint16_t duration);

void playChord(uint16_t* frequencies, uint16_t duration);

void playSong();

#endif /* LIB_H */