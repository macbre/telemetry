#ifndef _EEPROM_H
#define _EEPROM_H

#include "../telemetry.h"

//#define EEPROM_SS_PORT  PORTB
//#define EEPROM_SS_PIN   2

//unsigned char eeprom_current_bank;

// zestaw instrukcji EEPROMu 25xxxxxx
#define EEPROM_READ    0b00000011 // Read data from memory array beginning at selected address
#define EEPROM_WRITE   0b00000010 // Write data to memory array beginning at selected address
#define EEPROM_WREN    0b00000110 // Set the write enable latch (enable write operations)
#define EEPROM_WRDI    0b00000100 // Reset the write enable latch (disable write operations)
#define EEPROM_RDSR    0b00000101 // Read STATUS register
#define EEPROM_WRSR    0b00000001 // Write STATUS register
#define EEPROM_PE      0b01000010 // Page Erase – erase one page in memory array
#define EEPROM_SE      0b11011000 // Sector Erase – erase one sector in memory array
#define EEPROM_CE      0b11000111 // Chip Erase – erase all sectors in memory array
#define EEPROM_RDID    0b10101011 // Release from Deep power-down and read electronic signature
#define EEPROM_DPD     0b10111001 // Deep Power-Down mode

// inicjalizacja EEPROMu (konfiguracja I/O)
void eeprom_init(void);

// wybor aktualnego banku
//void eeprom_select_bank(unsigned char);

// odczyt i zapis pojedynczego bajtu
void eeprom_write(unsigned long, unsigned char);
unsigned char eeprom_read(unsigned long);

// pobranie stanu pamiêci
unsigned char eeprom_status();

// odblokowanie / zablokowanie zapisu (pamiêæ po uruchomieniu jest zablokowana przed zapisem!)
void eeprom_enable_write(unsigned char);

// odczyt i zapis stron (sekwencja maksymalnie 256 bajtów)
void eeprom_page_write(unsigned long, unsigned char*, unsigned char);
void eeprom_page_read(unsigned long, unsigned char*, unsigned char);

// kasowanie danych
void eeprom_page_erase(unsigned long);
void eeprom_sector_erase(unsigned long);
void eeprom_chip_erase();

// odczyt sygnatury
unsigned char eeprom_read_signature();

// detekcja rozmiaru wybranego banku
// @see AN690 Microchipa

// zwraca rozmiar aktualnie wybranego banku (w bajtach)
//  eeprom_get_size() >> 7  - wynik w kbitach
//  eeprom_get_size() >> 10 - wynik w kBajtach
unsigned long eeprom_get_size(void);

// bada czy pamiec ma podana pojemnosc N (w bajtach) - zakres adresow 0...(N-1)
unsigned char eeprom_is_size(unsigned long);

#endif
