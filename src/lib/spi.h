#ifndef __SPI_H
#define __SPI_H

#include "../telemetry.h"

// fizyczna konfiguracja portów SPI
//

// atmega8(8)
/*
#define SPI_PORT PORTB
#define SPI_SS_PIN   2
#define SPI_MOSI_PIN 3
#define SPI_MISO_PIN 4
#define SPI_SCK_PIN  5
*/
// atmega32
#define SPI_PORT PORTB
#define SPI_SS_PIN   4
#define SPI_MOSI_PIN 5
#define SPI_MISO_PIN 6
#define SPI_SCK_PIN  7


// inicjalizacja jako uk³adu master SPI
void spi_master_init(void);

// inicjalizacja jako uk³adu slave SPI
void spi_slave_init(void);

// odczyt/zapis SPI
unsigned char spi_read(void);
unsigned char spi_write(unsigned char data);

// odczyt w trybie burst
//void spi_burst_read(unsigned char, unsigned char*, unsigned int);

// wybór / deaktywacja wybranego uk³adu slave
#define spi_select(port, bit) 	    port &= ~_BV(bit); nop(); nop();	// !SS/CE = 0 (low)
#define spi_unselect(port, bit) 	port |= _BV(bit); nop(); nop(); 	// !SS/CE = 1 (hi)

// wybor czestotliwosci taktowania SPI (przy SPI2X)
#define spi_full_speed()            SPCR &= ~(0x03); SPCR |= (1<<SPI2X);                                // F_OSC/2  -> 8MHz
#define spi_medium_speed()          SPCR &= ~(0x03); SPCR |= (1<<SPR0)|(1<<SPI2X);                      // F_OSC/8  -> 2MHz
#define spi_low_speed()             SPCR &= ~(0x03); SPCR |= (1<<SPR1)|(1<<SPR0); SPSR &= ~(1<<SPI2X);  // F_OSC/128 -> 125 kHz

#endif
