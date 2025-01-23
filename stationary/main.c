#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 16000000UL
#define BAUD 1200
#define MYUBRR F_CPU/16/BAUD-1

// Password
#define PASSWORD "0000"
#define PASSWORD_LENTH 4

// Stepper config
#define STEPPER_STEPS 64
#define RANDOM_STEP ((rand() % (STEPPER_STEPS - 1)) + 1) // Генерация случайного числа от 1 до STEPPER_STEPS - 1, исключая 0

// PACKAGES
#define PACKAGE_COUNT 10
#define PACKAGE_SIZE 8

#include "stdio.h"
#include <stdlib.h>
#include "avr/io.h"
#include "lib/speaker.h"
#include "avr/interrupt.h"
#include "util/delay.h"
#include <avr/eeprom.h>
#include <stdbool.h>


typedef enum {
    RED,
    GREEN,
    YELLOW
} Color;

typedef enum {
    OFF_1,              // Во время выполнения отключен первый переключатель
    OFF_2,              // Во время выполнения отключен второй переключатель
    OFF_3,              // Во время выполнения отключен третий переключатель
    PING,               // Пинг мобильного модуля
    IDLE,               // Ожидание

    RECEIVING,          // 1-е задание, ожидание ввода пароля
    PASSWORD_RECEIVED,  // 1-е задание, пароль введен
    SENDING_PASSWORD,   // 1-е задание, отправка подсказки
    CHECKING_PASSWORD,  // 1-е задание, проверка пароля
    SUCCEED,            // 1-е задание, пароль правильный
    FAILED,             // 1-е задание, пароль неправильный

    SETUP_RADIO,        // 2-е задание, настройка радио
    RADIO_CONFIGURATED, // 2-е задание, настройка завершена

    SEND_PACKAGE_INFO,  // 3-e задание, хз как назвать
    SEND_PACKAGES,
} State;

volatile State currentState = IDLE; 
volatile uint8_t receivedData = 0;
// Пароль
volatile char receivedPassword[PASSWORD_LENTH]; 
volatile char password[PASSWORD_LENTH];
volatile uint8_t index = 0;
volatile bool passed_flag = false;

// Двигатель
volatile int currentPinIndex = 0;
volatile uint8_t pins[NUM_PINS] = PIN_ARRAY;
volatile bool fix = false;
volatile uint8_t prevButton = 0;
volatile uint8_t generatedStep = 1;
volatile uint8_t currentStep = 0;
volatile uint8_t steps = 64;

volatile uint8_t tryLeft = 3;

volatile Color ledColors[3] = {RED, RED, RED};

volatile bool switch1_processed = false;
volatile bool switch2_processed = false;
volatile bool switch3_processed = false;

void Buttons_Init();
void initPWM();
void Stepper_Init();

void changeColors(){
    PORT_LED ^= 1 << RESET_LED;
    PORT_LED ^= 1 << RESET_LED;
    setLEDColor(LED1, ledColors[0]);
    setLEDColor(LED2, ledColors[1]);
    setLEDColor(LED3, ledColors[2]);
}

void setLEDColor(uint8_t pin, Color color){
    switch(color) {
        case RED:
                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;
            break;
        case GREEN:
                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;

                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;
            break;
        case YELLOW:
                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;

                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;

                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;

                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;

                PORT_LED ^= 1 << pin;
                PORT_LED ^= 1 << pin;
            break;
    }
}

uint16_t generate_random_number(uint16_t seed) {
    seed *= 2654435761 % 4294967296;
    srand(seed);
    return RANDOM_STEP;
}

void leftRotate(){
    if (prevButton == 2){
        if (currentPinIndex == 3) currentPinIndex = 0;
        else currentPinIndex++;
        if (currentPinIndex == 3) currentPinIndex = 0;
        else currentPinIndex++;
        PORT_STEPPER |= 1<<pins[currentPinIndex];
        PORT_STEPPER &= 1<<pins[currentPinIndex];
        if (currentPinIndex == 3) currentPinIndex = 0;
        else currentPinIndex++;
    } else {
        PORT_STEPPER |= 1<<pins[currentPinIndex];
        PORT_STEPPER  &= 1<<pins[currentPinIndex];
        if (currentPinIndex == 3) currentPinIndex = 0;
        else currentPinIndex++;
    }
    prevButton = 1;
}

void rightRotate(){
    if (prevButton == 1){
        if (currentPinIndex == 0) currentPinIndex = 3;
        else currentPinIndex--;
        if (currentPinIndex == 0) currentPinIndex = 3;
        else currentPinIndex--;
        PORT_STEPPER  |= 1<<pins[currentPinIndex];
        PORT_STEPPER  &= 1<<pins[currentPinIndex];
        if (currentPinIndex == 0) currentPinIndex = 3;
        else currentPinIndex--;
    } else {
        PORT_STEPPER  |= 1<<pins[currentPinIndex];
        PORT_STEPPER  &= 1<<pins[currentPinIndex];
        if (currentPinIndex == 0) currentPinIndex = 3;
        else currentPinIndex--;
    }
    prevButton = 2;
}

ISR(INT0_vect){
    if (!fix) return;    
    if (!bit_is_set(DDR_BUTTON, BUTTON1)) { // D2
        leftRotate(); 
        generatedStep++;
        if (generatedStep == STEPPER_STEPS) {
            generatedStep = 0;
            USART_Transmit('='); 
            fix = false;
            currentState = RADIO_CONFIGURATED;

        }
        else USART_Transmit('+');  
    } 
}

ISR(INT1_vect){
    if (!fix) return;
    if (!bit_is_set(DDR_BUTTON, BUTTON2)){ // D3
        rightRotate(); 
        generatedStep--;
        if (generatedStep == 0) {
            USART_Transmit('=');
            fix = false;
            currentState = RADIO_CONFIGURATED;
        }
        else USART_Transmit('-');  
    }
}

ISR(PCINT2_vect){
     if (PCIFR & (1 << PCIF2)) { 
        if (!switch1_processed && (PIN_SWITCH & (1 << SWITCH1))) {
            currentState = PING;
            switch1_processed = true;
            USART_Transmit('+');
        } 
        if (switch1_processed && !(PIN_SWITCH & (1 << SWITCH1))){
            USART_Transmit('<');
            switch1_processed = false;
            currentState = OFF_1;
        }


        if (!switch2_processed && (PIN_SWITCH & (1 << SWITCH2))) {
            if (passed_flag){
                fix = true;
                currentState = SETUP_RADIO;
                switch2_processed = true;
            }
        } 
        if (switch2_processed && !(PIN_SWITCH & (1 << SWITCH2))){
            USART_Transmit('^');
            switch2_processed = false;
            currentState = OFF_2;
        }


        if (!switch3_processed && (PIN_SWITCH & (1 << SWITCH3))) { 
            currentState = SEND_PACKAGE_INFO;
            switch3_processed = true;
        } 
        if (switch3_processed && !(PIN_SWITCH & (1 << SWITCH3))){
            switch3_processed = false;
            currentState = OFF_3;

        }
     
    }
    PCIFR |= (1 << PCIF2);
}
 
void USART_Init( unsigned int ubrr ){
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0B |= (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); 
}

void USART_Transmit(unsigned char data){
    while (!(UCSR0A & (1 << UDRE0))); 
    UDR0 = data; 
}

void USART_TransmitString(const char* str){
    while(*str != '\0'){
        _delay_ms(100);
        USART_Transmit(*str);
        str++;
    }
}

ISR(USART_RX_vect) {
    receivedData = UDR0; 
    if (currentState == RECEIVING){
        if (receivedData != '\n') receivedPassword[index++] = receivedData;
        else {
            currentState = CHECKING_PASSWORD;
            index = 0;
        }
    }
}  

uint8_t compare(){
    for (int i = 0; i < PASSWORD_LENTH; i++){
        if (receivedPassword[i] != password[i]) return false;
    }
    return true;
}

void test1();

int main(void){
    for (int i = 0; i < PASSWORD_LENTH; i++){
        password[i] = PASSWORD[i];
    }
    uint16_t seed = eeprom_read_word((uint16_t*)SEED_ADDRESS);
    if((seed == 0)||(seed == 256)) {
        seed = 1; 
    }
    eeprom_write_word((uint16_t*)SEED_ADDRESS, seed+1);
    generatedStep = generate_random_number(seed);

    Buttons_Init();
    USART_Init(MYUBRR);
    Stepper_Init();
    initPWM(); 
    sei();
    changeColors();

    unsigned char buffer[20];
    int currentPinIndex = 0;
    uint16_t delay = 0; 
    char package[PACKAGE_SIZE + 1];

    for(;;){
        switch (currentState){
            case IDLE:
                break;
            case OFF_1:
                receivedData = 0;
                ledColors[0] = RED;
                ledColors[1] = RED;
                changeColors();
                passed_flag = false;
                fix = false;
                currentState = PING;
                break;
            case OFF_2:
                ledColors[1] = RED;
                changeColors();
                currentState = IDLE;
                break;
            case OFF_3:
                ledColors[2] = RED;
                changeColors();
                currentState = IDLE;
                break;
            case PING:
                if (receivedData == '+') {
                    ledColors[0] = YELLOW;
                    changeColors();
                    currentState = SENDING_PASSWORD;
                    receivedData = 0;
                }
                break;
            case SENDING_PASSWORD:
                test1();
                currentState = RECEIVING;
                break;
            case CHECKING_PASSWORD:
                if (compare()&&(tryLeft > 0)) {
                    currentState = SUCCEED;
                    passed_flag = true;
                }
                else currentState = FAILED;
                break;
            case SUCCEED:
                USART_Transmit('+');
                ledColors[0] = GREEN;
                changeColors();
                currentState = OFF_2;
                break;
            case FAILED:
                USART_Transmit('-');
                tryLeft--;
                currentState = RECEIVING;
                index = 0;
                break;
            case SETUP_RADIO:
                
                ledColors[1] = YELLOW;
                changeColors();

                sprintf(buffer, "%d", STEPPER_STEPS);
                USART_TransmitString(buffer);
                USART_Transmit('/');
                sprintf(buffer, "%d", generatedStep);
                USART_TransmitString(buffer);
                USART_Transmit('/');
                currentState = IDLE;
                if (generatedStep == 0) {
                    currentState = RADIO_CONFIGURATED;
                    break;
                }
                break;
            case RADIO_CONFIGURATED:
                ledColors[1] = GREEN;
                changeColors();
                currentState = IDLE;
                break;   
            case SEND_PACKAGE_INFO:
                ledColors[2] = YELLOW;
                changeColors();

                sprintf(buffer, "%d", PACKAGE_COUNT);
                USART_TransmitString(buffer);
                USART_Transmit('/');
                _delay_ms(100);
                sprintf(buffer, "%d", PACKAGE_SIZE);
                USART_TransmitString(buffer);
                USART_Transmit('/');


                currentState = SEND_PACKAGES;
                _delay_ms(1000);
                break;

            case SEND_PACKAGES:
                delay = (rand() % 1000) + 500;
                for(int i = 0; i < PACKAGE_COUNT; i++){
                    for(int j = 0; j < PACKAGE_SIZE; j++){
                        package[j] = 'A' + (random() % 26);
                    }
                    package[PACKAGE_SIZE] = '\0';
                    USART_TransmitString(package);
                    USART_Transmit('\0');
                    playSong();
                    _delay_ms(delay);
                }
                ledColors[2] = GREEN;
                changeColors();
                currentState = IDLE;
                break;
        }
    }
    return 0;
}


void Buttons_Init(){
    DDR_BUTTON &= ~((1<<BUTTON1)|(1<<BUTTON2));
    PORT_BUTTON |= (1<<BUTTON1)|(1<<BUTTON2);

    DDR_SWITCH &= ~((1<<SWITCH1) | (1<<SWITCH2) | (1<<SWITCH3));
    PORT_SWITCH |= (1<<SWITCH1) | (1<<SWITCH2) | (1<<SWITCH3);

    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);
    EICRA |= (1 << ISC11);
    EICRA &= ~(1 << ISC10);
    EIMSK |= (1 << INT0) | (1<<INT1);

    PCMSK2 |= (1 << PCINT21) | (1 << PCINT22)| (1 << PCINT23);
    PCICR |= (1 << PCIE0)|(1 << PCIE2);
}

void Stepper_Init(){
    for (int i = 0; i < NUM_PINS; i++){
        DDR_STEPPER |= 1<<pins[i];
        PORT_STEPPER &= 1<<pins[i];
    }
}

void initPWM() {
    TCCR1A |= (1 << WGM10);
    TCCR1B |= (1 << WGM12) | (1 << CS10); // Без предделителя (для F_CPU = 8MHz)
    DDRB |= (1 << SPEAKER_PIN); // Установка пина PB1 на вывод
}

void test1(){
    _delay_ms(100);
    USART_Transmit('1');
    _delay_ms(100);
    USART_TransmitString(password);
    _delay_ms(100);
    USART_Transmit('\n');
};
