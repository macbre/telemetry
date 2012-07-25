#ifndef _DS1306_H
#define _DS1306_H

#include "../telemetry.h"

//#define RTC_SS_PORT     PORTB
//#define RTC_SS_PIN      0

// inicjalizacja RTC
void ds1306_init(void);

// odczyt i zapis bajtu
unsigned char ds1306_read(unsigned char);
void ds1306_write(unsigned char, unsigned char);

// funkcje ustawiania/pobierania aktualnego czasu
void ds1306_time_set(time_t*);
void ds1306_time_get(time_t*);

#endif
