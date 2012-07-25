#ifndef _FIRMWARE_H
#define _FIRMWARE_H

/**
    Pamiec EEPROM (128 kB) zostala podzielona na 1 kB sektory (4 "fizyczne" strony po 256 bajtow).
    Pierwszy sektor (0x0000 - 0x03ff) zawiera tablice wpisow o dlugosci danych kazdego
    z pozostalych sektorow (0x0400-) (-> firmware_get_sector_length).
**/

#include "../telemetry.h"

#define FIRMWARE_PORT       1500

#define FIRMWARE_OP_SEND    1
#define FIRMWARE_OP_VERIFY  2
#define FIRMWARE_OP_SEND_OK 3

//#define FIRMWARE_SECTOR_SIZE 1024 // 4 strony * 256 bajtów
#define FIRMWARE_SECTOR_SIZE 1280   // 5 stron  * 256 bajtów

typedef struct {
    unsigned char op;
    unsigned long offset;
    unsigned char len;
    unsigned char data[];
} /* 6 */ firmware_packet;

unsigned int firmware_handle_packet(unsigned char*);
unsigned int firmware_get_sector_length(unsigned char);
unsigned int firmware_read_sector(unsigned char, unsigned char*);

#endif
