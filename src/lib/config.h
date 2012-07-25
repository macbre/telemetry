#ifndef _CONFIG_H
#define _CONFIG_H

#include "../telemetry.h"

// struktura ustawieñ w pamiêci EEPROM Atmegi
typedef struct {
    // nag³ówek -> weryfikacja poprawnoœci zapisanych danych w EEPROM
    unsigned char header;
    
    // warsta sieciowa
    uint8_t       my_mac[6];
    uint8_t       my_ip[4];
    uint8_t       gate_ip[4];
    uint8_t       mask[4];

    // przypisania czujników DS18B20 do kana³ów pomiarowych
    // patrz komentarz w ds18b20.c
    unsigned char ds_assignment[DS_DEVICES_MAX];

    // ustawienia typu tak/nie (maska bitowa)
    unsigned int  config;
} config;

// odczyt / zapis konfiguracji
unsigned char config_read(config*);
unsigned char config_save(config*);

// menu konfiguracyjne w terminalu
void config_menu();

// czeka na odpowiedŸ typu (tak/nie) od u¿ytkownika
unsigned char config_ask();

// czeka na wprowadzenie adresu IP
void config_ip(uint8_t* ip);

// zwraca wybran¹ cyfrê
unsigned char config_get_num();

// nag³ówek struktury (zmieñ wartoœæ po zmianie struktury typu config)
#define CONFIG_HEADER   0xA2

// maska bitowa na pole config
#define CONFIG_USE_DHCP 1

// ustawienia systemu
config my_config;

#endif
