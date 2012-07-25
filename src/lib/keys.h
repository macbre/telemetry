#ifndef _KEYS_H_
#define	_KEYS_H_

#include "../telemetry.h"

unsigned char keys_state, keys_pressed_mask;

// inicjalizacja klawiatury
void keys_init();

// skanuj stan klawiatury
void keys_scan();

// pobierz stan podanego przycisku
unsigned char keys_get(unsigned char);

// czy wciœniêto jakikolwiek przycisk?
unsigned char keys_pressed();

#endif
