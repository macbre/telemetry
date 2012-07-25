#ifndef _PWM_H
#define _PWM_H

#include "../telemetry.h"

// liczba kana³ów PWM
#define PWM_CHANNELS    8

// wartosci wysterowan wyjsc PWM
volatile unsigned char pwm_fill[8];
volatile unsigned char pwm_fill_buf[8];

// krok cyklu PWM
volatile unsigned char pwm_step;

// inicjalizacja sterownika PWM
void pwm_init();

// ustawienie wypelnienia wybranego kanalu PWM
void pwm_set_fill(unsigned char, unsigned char);

// pobranie wypelnienia wybranego kanalu PWM
#define pwm_get_fill(ch) pwm_fill[(ch)]

// pojedynczy przebieg sterownika PWM - aktualizacja wyjsc
void pwm_loop();

#endif
