#ifndef LIB_H
#define LIB_H

#define RS PC5
#define RW PC4 
#define EN PC6 
#define CTRLPORT PORTC
#define DATAPORT PORTC
#define DDRDATA DDRC
#define DDRCTRL DDRC

#include "stdio.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include "util/delay.h"

// Функция записи команды в LCD
void lcd_com(unsigned char p);
// Функция записи данных в LCD
void lcd_dat(unsigned char p);
// Функция инициализации LCD
void lcd_init(void);

void lcd_print_string(const char *str);

void lcd_print_int(int number);

#endif /* LIB_H */