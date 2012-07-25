#include "ds18b20.h"

// detekcja czujnikow temperatury ds18b20 (family code = 0x28)
unsigned char ds_devices[DS_DEVICES_MAX][8];    // kody ROM czujników
unsigned char ds_devices_count;                 // liczba wykrytych czujników 1wire
volatile signed int ds_temp[DS_DEVICES_MAX];    // tablica na aktualnie zmierzone temperatury (gdzie wartosc 227 odpowiada 22.7C)

void ds18b20_init(unsigned char resolution) {
    
    ds_devices_count = 0;

    // detekcja slave'ow na 1wire
    if (ow_first_search() == 1) {
        do {
            unsigned char b;

            // dopisz do listy czujnikow ds18b20
            if ( (OW_ROM[0] == 0x28) && (ds_devices_count < DS_DEVICES_MAX) ) {
                for (b=0; b<8; b++) {
                    ds_devices[ds_devices_count][b] = OW_ROM[b];
                    ds_temp[ds_devices_count] = 0;
                }

                ds_devices_count++; // zwieksz licznik liczby czujnikow ds18b20
            }

        } while (ow_next_search() == 1);

        // ustaw rozdzielczosc pomiaru temperatur i zazadaj pierwszego pomiaru
        //ds18b20_set_resolution(DS18B20_RESOLUTION_11_BITS); // 0.125C
        //ds18b20_set_resolution(DS18B20_RESOLUTION_12_BITS); // 0.0625C
        ds18b20_set_resolution(resolution);
        ds18b20_request_measure();
    }
}

// wysy³a wszystkim czujnikom na magistrali 1wire
// zadanie dokonania pomiaru temperatury i zapisu
// wyniku w pamieci Scrachpad
void ds18b20_request_measure()
{
    ow_reset();
    ow_write(0xcc); // skip ROM (zadanie do wszystkich)
    ow_write(0x44); // pomiar

    return;
}

// pobierz temperature z podanego czujnika
//
// np. temp. 25.4C odczytywana jest jako 254 (zakres odczytow (-550 ; +1250)
//
// odczyt 2000 oznacza b³¹d!
//
signed int ds18b20_get_temperature(unsigned char* dev)
{
    unsigned int tmp;   // zmienna pomocnicza przy odczycie rejestrow ds18b20
    signed int temp;   // temperatura

    ow_reset();

    ow_match_rom(dev);

    ow_write(0xbe); // read scratchpad

    // odczytaj temperature z rejestrow ds18b20
    tmp = ow_read() | ((unsigned int)ow_read() << 8);

    ow_reset(); // odczytalismy juz potrzebne dane (niech ds18b20 nie przesyla wiecej danych)

    // b³¹d na szynie
    if ( (tmp == 0xffff) || (tmp == 0x0000) ) {
        return 2000;
    }

    // test
    //tmp = 0xfc90;   // -55
    //tmp = 0xfe6f;   // -25.0625
    //tmp = 0xff5e;   // -10.125
    //tmp = 0xfff8;   // -0.5
    //tmp = 0x0008;   // 0.5
    //tmp = 0x00a2;   // 10.125
    //tmp = 0x0191;   // 25.0625
    //tmp = 0x07D0;   // 125

    //
    // przelicz temperature
    //

    // najpierw wartosci calkowita stopni
    if (tmp >> 12 == 0x0f) {
        temp = ((0xffff - tmp + 0x00001) >> 4); // temperatura ujemna -> przelicz
    }
    else {
        temp = (tmp >> 4);
    }

    temp *= 10;

    // czesc ulamkowa
    temp += (tmp & 0b1000) ? 5 : 0; // 0.5
    temp += (tmp & 0b0100) ? 3 : 0; // 0.25
    temp += (tmp & 0b0010) ? 2 : 0; // 0.125
    temp += (tmp & 0b0001) ? 1 : 0; // 0.0625


    if (tmp >> 12 == 0x0f) {
        temp *= -1; // temperatura ujemna -> przelicz
    }

    return temp;
}

// pobierz temperature z wszystkich czujnikow DS18B20
void ds18b20_get_temperature_from_all()
{
    signed int tmp;

    // przypisania kana³ów do kolejnych czujników DS18B20 na szynie 1wire
    //
    //
    // !!! Identyfikacja konieczna po ka¿dej zmianie lub dodaniu czujników !!!
    //
    //
    // np. czujnik wykryty jako drugi na szynie 1wire przypisany do kana³u #4
    //     czujnik wykryty jako trzeci na szynie 1wire przypisany do kana³u #2
    //     ...
    //
    // #define DS_ASSIGNMENT   {1,4,2,0,3}

    // odczyt temperatury z kolejnych czujnikow
    // temperatura 25.3 zostanie zapisana jako wartosci 253
    for (unsigned int dev=0; dev<ds_devices_count; dev++) {
        tmp = ds18b20_get_temperature(ds_devices[ my_config.ds_assignment[dev] ]);
    
        if (tmp != 2000)
            ds_temp[dev] = tmp;
    }
}


// ustaw rozdzielczosc pomiaru temperatur
// wszystkich czujnikow
void ds18b20_set_resolution(unsigned char res)
{
    unsigned char conf = 0b00011111;

    conf |= res << 5;

    ow_reset();

	ow_write(0x4E); // konfiguracja DS18B20
	ow_write(0x00); // T_l
	ow_write(0x00); // T_h
	ow_write(conf); // bajt konfiguracyjny

    ow_reset();

    return;
}

// ustaw zakresy wyzwalania alarmow podanego czujnika
void ds18b20_set_triggers(unsigned char* dev, signed char tl, signed char th)
{
    ow_reset();

    ow_match_rom(dev);    

	ow_write(0x4E); // konfiguracja DS18B20
	ow_write(0x00); // T_l
	ow_write(0x00); // T_h
    ow_reset();     // nie przesylamy juz bajtu konfiguracyjnego
}
