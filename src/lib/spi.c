#include "spi.h"

void spi_master_init(void)
{
	// ustaw kierunki dla pinow SPI
	DDR(SPI_PORT) |=  (1<<SPI_SCK_PIN) | (1<<SPI_MOSI_PIN) | (1<<SPI_SS_PIN);   // wyjœcia (MOSI,SCK,SS)
	
    // podwojna predkosc SPI w trybie Master - taktowanie F_OSC/2
    SPSR = (1<<SPI2X);

    SPCR = 0;

    // taktowanie SPI - 8MHz
    spi_full_speed();

    //SPCR |= (1<<SPR0);           // taktowanie F_OSC / 16
    //SPCR |= (1<<SPR0)|(1<<SPR1); // taktowanie F_OSC / 128

    SPCR |= (1<<SPE)|(1<<MSTR);  // w³¹cz SPI jako uk³ad master
}

void spi_slave_init(void)
{
	// ustaw kierunki dla pinow SPI
	DDR(SPI_PORT) |=  (1<<SPI_MISO_PIN); //wyjœcia MISO
	
    SPCR = (1<<SPE);  // w³¹cz SPI jako uk³ad slave
}

unsigned char spi_read(void)
{
    SPDR = 0xff; // dummy

	// oczekiwanie na zakonczenie transmisji
	while(!(SPSR&=~(1<<SPIF)));

    // zerowanie znacznika transmisji
	SPSR &= ~(1<<SPIF);

    // odczytaj stan rejestru
    return SPDR;
}

unsigned char spi_write(unsigned char data)
{
	SPDR = data;

	// oczekiwanie na zakonczenie transmisji
	while(!(SPSR&=~(1<<SPIF)));

	// zerowanie znacznika transmisji
	SPSR &= ~(1<<SPIF);

    return SPDR;
}
