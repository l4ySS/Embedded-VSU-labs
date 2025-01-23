#include "speaker.h"
uint16_t C_chord[] = {261, 330, 392}; 
uint16_t G_chord[] = {392, 494, 587}; 
uint16_t Am_chord[] = {220, 293, 392}; 
uint16_t F_chord[] = {349, 440, 523}; 
uint16_t duration = 1000; 

void playNote(uint16_t frequency, uint16_t duration) {
    uint16_t period = 1000000UL / frequency; // Период в микросекундах
    uint16_t pulseWidth = period / 2; // Ширина импульса (50% duty cycle)
    uint16_t cycles = frequency * duration / 1000; // Количество циклов для воспроизведения ноты

    for (uint16_t i = 0; i < cycles; i++) {
        PORT_SPEAKER |= (1 << SPEAKER_PIN); // Установка пина PD5 в HIGH
        _delay_us(pulseWidth);
        PORT_SPEAKER &= ~(1 << SPEAKER_PIN); // Установка пина PD5 в LOW
        _delay_us(pulseWidth);
    }
}

void playChord(uint16_t* frequencies, uint16_t duration) {
    for (int i = 0; i < 3; i++) {
        playNote(frequencies[i], duration);
    }
}

void playSong(){
    playChord(C_chord, duration);
    _delay_ms(100); 
    playChord(G_chord, duration);
    _delay_ms(100);
    playChord(Am_chord, duration);
    _delay_ms(100);
    playChord(F_chord, duration);
    _delay_ms(100);
}