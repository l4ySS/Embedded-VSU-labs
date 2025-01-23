#define F_CPU 16000000UL
#define BAUD 1200
#define MYUBRR F_CPU/16/BAUD-1
#include "lib/lcd.h"
#include <avr/eeprom.h>

  
#define KEYPAD_PORT_OUT PORTB  // Порт для столбцов
#define KEYPAD_PORT_IN PINB    // Порт для строк
#define KEYPAD_DDR_OUT DDRB    // Регистр направления для столбцов
#define KEYPAD_DDR_IN DDRB     // Регистр направления для строк


typedef enum {
    PING,
    IDLE,
    OFF_1,
    OFF_2,
    OFF_3,

    ENTER_PASSWORD,
    RECEIVING,
    PASSWORD_RECEIVED,
    COMMAND_COMPLETED,
    PASSWORD_SENDED,
    SUCCEED,
    FAILED,
    BLOCK,

    GET_POS,
    SETUP_RADIO,
    CONFIGURATING_RADIO,
    RADIO_CONFIGURATED,

    WAITING_SWITCH3,
    WAITING_PACKAGE_COUNT,
    WAITING_PACKAGE_SIZE,
    RECEIVING_PACKAGES,
    PRINT_PACKAGES,
    SAVE_PACKAGE,
    PRINT_RESULT
} State;

volatile uint8_t tryLeft = 3;
volatile State currentState = PING;
volatile uint8_t receivedData = 0;
volatile uint8_t rotate = 0;

volatile char receivedPassword[10];
volatile char enteredPassword[10];
volatile uint8_t index = 0;
volatile uint8_t passwordLenth = 0;

volatile uint8_t teeth = 0;
volatile uint8_t position = 0;
volatile uint8_t currentPos = 0;

// Packages
volatile uint8_t packageCount = 0;
volatile uint8_t packageNumber = 0;
volatile uint8_t packageSize = 0;

volatile uint8_t downloadSymbol = 0; // packageCount / 10;
volatile uint8_t totalSymbols = 0; // количество полос загрузки
volatile uint8_t package[10];
volatile uint8_t i = 0;
volatile uint8_t receivedPackages = 0;



void USART_Init( unsigned int ubrr ){
    UBRRH = (unsigned char)(ubrr>>8);
    UBRRL = (unsigned char)ubrr;
    UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
    UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}

void USART_Transmit(unsigned char data){
    while (!(UCSRA &(1<<UDRE)));
    UDR = data;
}

void USART_TransmitString(const char* str){
    while(*str != '\0'){
        _delay_ms(100);
        USART_Transmit(*str);
        str++;
        }
}

ISR(USART_RXC_vect){
    receivedData = 0;
    receivedData = UDR;
    if (receivedData == '^') {
        currentState = SUCCEED;
    }
    if (receivedData == '<') {
        currentState = PING;
    }
    switch(currentState){
        case PING:
            if (receivedData == '+') {
                USART_Transmit('+');
                currentState = IDLE;
                tryLeft = 3;
            }
            break;
        case IDLE:
            if (receivedData == '1') {
                currentState = RECEIVING;
            }
            break;
        case RECEIVING:
            if (receivedData != '\n') receivedPassword[index++] = receivedData;
            else {
                currentState = PASSWORD_RECEIVED;
                passwordLenth = index;
                index = 0;
                receivedData = 0;
            }
            break;
        case PASSWORD_SENDED:
            if (receivedData == '+') {
                currentState = SUCCEED;
            }
            if (receivedData == '-') currentState = FAILED;
            break;
        case SUCCEED:
            if (receivedData >= '0' && receivedData <= '9') {
                teeth = teeth * 10 + (receivedData - '0');
            }
            else {
                currentState = GET_POS;
            }
            break;
        case GET_POS:
            if (receivedData >= '0' && receivedData <= '9') {
                position = position * 10 + (receivedData - '0');
            }
            else {
                currentState = SETUP_RADIO;
            }
            break;
        case CONFIGURATING_RADIO:
            if (receivedData == '+') {
                rotate = 1;
                currentState = SETUP_RADIO;
            }
            if (receivedData == '-') {
                rotate = 2;
                currentState = SETUP_RADIO;
            }
            if (receivedData == '=') {
                rotate = 0;
                currentState = RADIO_CONFIGURATED;

            }
            break;
        case WAITING_PACKAGE_COUNT:
            if (receivedData >= '0' && receivedData <= '9') {
                packageCount = packageCount * 10 + (receivedData - '0');
            } else {
                lcd_com(0x01);
                lcd_print_string("Count: ");
                if (packageCount >= 10 ) {
                    downloadSymbol = packageCount / 10;
                } else {
                    downloadSymbol = 10 / packageCount;
                }
                currentState = WAITING_PACKAGE_SIZE;
            }
            break;
        case WAITING_PACKAGE_SIZE:
            if (receivedData >= '0' && receivedData <= '9') {
                packageSize = packageSize * 10 + (receivedData - '0');
            } else {
                lcd_com(0x01);
                lcd_print_string("SIZE: ");
                currentState = PRINT_PACKAGES;
            }
            break;

        case RECEIVING_PACKAGES:
            if (receivedData != '\0') {
                package[i] = receivedData;
                i++; 
            } else {
                USART_TransmitString(package);
                receivedPackages++;
                currentState = PRINT_PACKAGES;
                i=0;
                if ((packageCount >= 10) && (receivedPackages % downloadSymbol == 0)){
                    totalSymbols++;
                } else {
                    if (packageCount < 10) totalSymbols+=downloadSymbol;
                }
            }
            break;
    }   
}

void writeStringToEEPROM(uint8_t* str, uint16_t addr) {
    while (*str != '\0') {
        eeprom_write_byte((uint8_t*)addr++, *str++);
    }
    eeprom_write_byte((uint8_t*)addr, '\0'); 
}

void eraseEEPROM() {
    for (uint16_t addr = 0; addr < E2END; addr++) {
        eeprom_write_byte((uint8_t*)addr, 0xFF); 
        eeprom_busy_wait();
    }
}

void printPassword(){
    _delay_ms(2000);
    lcd_com(0x01);
    lcd_dat(receivedPassword[0]);
    lcd_dat(receivedPassword[1]);
    lcd_dat(receivedPassword[2]);
    lcd_dat(receivedPassword[3]);
    lcd_dat(' ');
    lcd_dat(' ');
    lcd_dat(' ');
    lcd_dat(' ');
    lcd_dat(' ');
    lcd_dat(' ');
    for (uint8_t i = 0; i < tryLeft; i++) lcd_dat('!');
    lcd_com(0xC0);
    lcd_print_string("Enter:");
}

void printStepper(){
    _delay_ms(1);
    lcd_com(0x01);
    lcd_print_string("Configurating:");
    lcd_com(0xC0);
    lcd_print_int(position);
}

void printPackagesInfo(){
    _delay_ms(1);
    lcd_com(0x01);
    lcd_print_string("Count:");
    lcd_print_int(packageCount);
    lcd_print_string(" Size:");
    lcd_print_int(packageSize);
    lcd_com(0xC0);

    lcd_dat('[');
    for (int i = 0; i < totalSymbols; i++) lcd_dat('|');
    for (int i = totalSymbols; i < 10; i++) lcd_dat(' ');
    lcd_dat(']');
}

void printResult(){
     _delay_ms(1);
    lcd_com(0x01);
    lcd_print_string("Received:");
    lcd_com(0xC0);
    lcd_print_int(receivedPackages);
    lcd_dat('/');
    lcd_print_int(packageCount);
}

void keypad_init() {
    KEYPAD_DDR_OUT |= 0x70; // PB4-PB6
    KEYPAD_DDR_IN &= 0x0F;  // PB0-PB3
}


char keypad_scan() {
    char key = 0; 

    // Проходим по каждому столбцу
    for (int i = 4; i < 7; i++) { 
        // Включаем текущий столбец
        KEYPAD_PORT_OUT |= (1 << i);
        // Проходим по каждой строке и проверяем, есть ли нажатие
        for (int j = 0; j < 4; j++) { 
            if ((KEYPAD_PORT_IN & (1 << j))) {
                 if (i == 4) {
                    if (j == 0) key = '1';
                    else if (j == 1) key = '4';
                    else if (j == 2) key = '7';
					else if (j == 3) key = '*';
                } else if (i == 5) {
                    if (j == 0) key = '2';
                    else if (j == 1) key = '5';
                    else if (j == 2) key = '8';
                    else if (j == 3) key = '0';
                } else if (i == 6) {
                    if (j == 0) key = '3';
                    else if (j == 1) key = '6';
                    else if (j == 2) key = '9';
					else if (j == 3) key = '#';
                }
                break; 
            }
        }
        // Выключаем текущий столбец
        KEYPAD_PORT_OUT &= ~(1 << i);
        if (key) break;
    }

    return key; 
}

int main (void)
{
keypad_init();
lcd_init(); 
//eraseEEPROM();
USART_Init(MYUBRR);
sei();

lcd_print_string("IDLE");
lcd_com(0xC0);
char key = 0;
static uint16_t eepromAddr = 0; // Адрес в EEPROM для записи

for(;;){
    switch(currentState){
        case PING:
            lcd_com(0x01);
            lcd_print_string("IDLE");
            lcd_com(0xC0);
            break;
        case RECEIVING:
            lcd_com(0x01);
            lcd_print_string("Initializing..."); 
            break;
        case PASSWORD_RECEIVED:
            printPassword();
            currentState = ENTER_PASSWORD;
            break;
        case ENTER_PASSWORD:
            key = keypad_scan();
            if ((key)&&(index < passwordLenth)&&(key!='#')) {
                lcd_dat('*');
                enteredPassword[index++] = key;
            }
            if ((key == '#')&&(index >= passwordLenth-1)) {
                USART_TransmitString(enteredPassword);
                _delay_ms(100);
                USART_Transmit('\n');
                index = 0;
                currentState = PASSWORD_SENDED;
            }
            key = 0;
            break;
        case PASSWORD_SENDED:
            lcd_com(0x01);
            lcd_print_string("Please, wait...");
            break;
        case SUCCEED:
            lcd_com(0x01);
            lcd_print_string("Correct");
            lcd_com(0xC0);
            lcd_print_string("Waiting Switch2");
            break;
        case FAILED:
            lcd_com(0x01);
            lcd_print_string("Failed");
            tryLeft--;
            _delay_ms(1000);
            if (tryLeft == 0) currentState = BLOCK;
            else currentState = PASSWORD_RECEIVED;
            break;
        case SETUP_RADIO:
            currentState = CONFIGURATING_RADIO;

            if (rotate == 1) {
                position++;
                if (position == teeth) position = 0;
            }
            if (rotate == 2) {
                position--;
                if (position == 0) position = teeth;
            }

            printStepper();

            break;
        case RADIO_CONFIGURATED:
            lcd_com(0xC0);
            lcd_print_string("SUCCEED");
            _delay_ms(1000);
            currentState = WAITING_SWITCH3;
            break;
        case WAITING_SWITCH3:
            lcd_com(0x01);
            lcd_print_string("Waiting Switch3");
            currentState = WAITING_PACKAGE_COUNT;
            break;
        case BLOCK:
            lcd_com(0x01);
            lcd_print_string("BLOCK");
            break;

        case PRINT_PACKAGES:
            printPackagesInfo();
            currentState = SAVE_PACKAGE;
            break;
        
        case SAVE_PACKAGE:
            if (package[0] != '\0'){
                writeStringToEEPROM(package, eepromAddr);
                eepromAddr += packageSize+1; // Увеличение адреса для следующей записи
            }
            if (receivedPackages == packageCount) currentState = PRINT_RESULT;
            else currentState = RECEIVING_PACKAGES;
            break;

        case PRINT_RESULT:
            _delay_ms(2000);
            printResult();
            break;
        
    }
	
	_delay_ms(100);

}
}

