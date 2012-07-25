#ifndef _WEBPAGE_H
#define _WEBPAGE_H

#include "../telemetry.h"

// sygnatura serwera
extern const char WEBPAGE_SERVER[] PROGMEM;

// porty na przychodzace zadania HTTP (glowny i rezerwowy)
#define WEBPAGE_PORT      80
#define WEBPAGE_PORT_ALT  8080

// obsluguje zadanie HTTP zwracajac tresc pakietu zwrotnego i jego dlugosc
unsigned int webpage_handle_http(void*, unsigned char*);

// pobierz zawartosc statyczna (/static/...)
unsigned int webpage_get_static_content(char*, void*);

//
// zawartoœæ dynamiczna
//

// pobierz stronê powitalna
unsigned int webpage_get_welcome_page(void*);

// pobierz stronê ustawieñ
unsigned int webpage_get_setup_page(void*);

// pobierz stronê PWM
unsigned int webpage_get_pwm_page(void*);

// pobierz stronê informacyjn¹
unsigned int webpage_get_info_page(void*);

// pobierz stronê identyfikacji parametrów systemu
unsigned int webpage_get_ident_page(void*);

// pobierz stronê akwizycji danych
unsigned int webpage_get_daq_page(void*);

// lista plików z pomiarami
unsigned int webpage_get_daq_list(void*);

// pobierz plik z wynikami pomiarów
unsigned int webpage_get_daq_data(char*, void*);

// wstaw nag³ówek
unsigned int webpage_print_header(void*, unsigned int);

// wstaw stopkê
unsigned int webpage_print_footer(void*, unsigned int);



// pobierz zawartosc generowana dynamicznie (/json/...)
unsigned int webpage_get_json_content(char*, void*);

#endif
