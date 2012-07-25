#ifndef _DS18B20_H
#define _DS18B20_H

#include "../telemetry.h"

extern unsigned char ds_devices[DS_DEVICES_MAX][8]; // kody ROM czujników
extern unsigned char ds_devices_count;              // liczba wykrytych czujników 1wire
extern volatile signed int ds_temp[DS_DEVICES_MAX]; // tablica na aktualnie zmierzone temperatury (gdzie wartosc 227 odpowiada 22.7C)

// inicjalizacja czujnikow DS18B20 z zadana rozdzielczoscia pomiarow
// wykonuje skanowanie magistrali 1wire,
// konfiguracje czujnikow i odczyt ich kodow ROM
void ds18b20_init(unsigned char);

// wysy³a wszystkim czujnikom na magistrali 1wire
// zadanie dokonania pomiaru temperatury i zapisu
// wyniku w pamieci Scrachpad
void ds18b20_request_measure();

// pobierz temperature z podanego czujnika
signed int ds18b20_get_temperature(unsigned char*);

// pobiera temperature z wszystkich czujnikow DS18B20
// podlaczonych do magistrali i zapisuje wyniki do
// tablicy ds_temp
void ds18b20_get_temperature_from_all();

// ustaw rozdzielczosc pomiaru temperatur
// wszystkich czujnikow
void ds18b20_set_resolution(unsigned char);

// ustaw zakresy wyzwalania alarmow podanego czujnika
void ds18b20_set_triggers(unsigned char*, signed char, signed char);

// rozdzielczoœci                           // precyzja     maksymalny czas pomiaru
#define DS18B20_RESOLUTION_9_BITS    0b00   // 0.5C         93.75 ms
#define DS18B20_RESOLUTION_10_BITS   0b01   // 0.25C        187.5 ms
#define DS18B20_RESOLUTION_11_BITS   0b10   // 0.125C       375 ms
#define DS18B20_RESOLUTION_12_BITS   0b11   // 0.0625C      750 ms

#endif
