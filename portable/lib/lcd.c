
#include "lcd.h"


void lcd_com(unsigned char data){
CTRLPORT &= ~(1 << RS); 

CTRLPORT |= (1 << EN); 
DATAPORT = (DATAPORT & 0xF0) | ((data >> 4) & 0x0F); 
_delay_us(1); 
CTRLPORT &= ~(1 << EN); 

CTRLPORT |= (1 << EN); 
DATAPORT = (DATAPORT & 0xF0) | (data & 0x0F); 
_delay_us(1); 
CTRLPORT &= ~(1 << EN); 
_delay_us(100); 
}
  
void lcd_dat(unsigned char data){
CTRLPORT |= (1 << RS); 

CTRLPORT |= (1 << EN); 
DATAPORT = (DATAPORT & 0xF0) | ((data >> 4) & 0x0F); 
_delay_us(1);
CTRLPORT &= ~(1 << EN); 

CTRLPORT |= (1 << EN);
DATAPORT = (DATAPORT & 0xF0) | (data & 0x0F); 
_delay_us(1); 
CTRLPORT &= ~(1 << EN); 
_delay_us(100); 
}

void lcd_print_string(const char *str) {
    while (*str != '\0') {
        lcd_dat(*str);
        str++;
    }
}  

void lcd_print_int(int number) {
    char buffer[20]; 
    sprintf(buffer, "%d", number); 
    for (int i = 0; buffer[i] != '\0'; i++) {
        lcd_dat(buffer[i]); 
    }
}

void lcd_init(void){
DDRCTRL |= (1 << EN)|(1 << RS); 
CTRLPORT = 0x00; 
DDRDATA = 0xF0; 
DATAPORT = 0x00; 
lcd_com(0x08);
lcd_com(0x38);
lcd_com(0x38); 
lcd_com(0x32); 
lcd_com(0x28); // 5 на 8 один каждый символ
lcd_com(0x01); // Очистка дисплея
lcd_com(0x06); // Сдвиг курсора вправо
lcd_com(0x0C); // Курсор невидим
}
