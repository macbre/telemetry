#ifndef _1WIRE_H
#define _1WIRE_H

#include "../telemetry.h"

// makra dla operacji na "magistrali" 1wire
#define OW_ONE          OW_PORT |= 1<<OW_PIN
#define OW_ZERO         OW_PORT &= ~(1<<OW_PIN)
#define OW_OUTPUT       DDR(OW_PORT) |= 1<<OW_PIN
#define OW_INPUT        DDR(OW_PORT) &= ~(1<<OW_PIN)
#define OW_READ         (PIN(OW_PORT) & (1<<OW_PIN))

// makra dla opoznien z noty AN126 Maxima
// AVR-GCC: "The maximal possible delay is 768 us / F_CPU in MHz"
//  8 MHz -> 96 us
// 16 MHz -> 48 us
//
#define OW_DELAY_A 6
#define OW_DELAY_D 10
#define OW_DELAY_E 9
#define OW_DELAY_G 0

// _delay_loop_2 - 1 iteracja = 4 takty = (4/MHz) us
#define OW_MHZ  (F_CPU / 1000000UL)

#define OW_DELAY_B (64  /4 * OW_MHZ)
#define OW_DELAY_C (60  /4 * OW_MHZ)
#define OW_DELAY_F (55  /4 * OW_MHZ)
#define OW_DELAY_H (480 /4 * OW_MHZ)
#define OW_DELAY_I (70  /4 * OW_MHZ)
#define OW_DELAY_J (410 /4 * OW_MHZ)

// reset magistrali 1wire (zwraca informacje czy na magistrali dziala slave)
unsigned char ow_reset();

// odczyt/zapis pojedynczych bitow
unsigned char ow_read_bit();
void ow_write_bit(unsigned char);

// odczyt/zapis bajtow
unsigned char ow_read();
void ow_write(unsigned char);

// odczytaj kod ROM (tylko dla jednego slave'a!)
void ow_read_rom_code(unsigned char*);

// wybierz (Match ROM) slave'a na magistrali 1wire
void ow_match_rom(unsigned char*);

// szukanie urzadzen 1wire (kodow ROM)
unsigned char OW_ROM[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;

// znajdz pierwsze urzadzenie 1wire 
unsigned char ow_first_search();

// znajdz nastepne urzadzenie 1wire 
unsigned char ow_next_search();     

// znajdz urzadzenia 1wire
unsigned char ow_search();   

#endif
