#include "eeprom.h"

void eeprom_init(void)
{
    // piny dla !CS jako wyjœcia
    DDR(EEPROM_SS_PORT) |= (1<<EEPROM_SS_PIN);      // bank #0
    DDR(EEPROM_SS_PORT) |= (1<<(EEPROM_SS_PIN+1));  // bank #1

    // ustaw na CS stan wysoki
    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);
    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN + 1);
}

unsigned char eeprom_status()
{
    spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_RDSR);    // read status register

        // wyœlij bajt do zapisania
        unsigned char status = spi_read();

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    return status;
}

void eeprom_enable_write(unsigned char enable)
{
    spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write( (enable == 1 ? EEPROM_WREN : EEPROM_WRDI) );    // enable / disable write

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);
}

void eeprom_write(unsigned long addr, unsigned char data)
{
     spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_WRITE);    // byte write

        // wyœlij adres 24-bitowy
        spi_write( (addr >> 16) & 0xff);
        spi_write( (addr >> 8)  & 0xff);
        spi_write(  addr        & 0xff);

        // wyœlij bajt do zapisania
        spi_write(data);
        

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    //_delay_ms(2);_delay_ms(2);_delay_ms(2);_delay_ms(2); // Twc = 6ms

    // czekaj na zakoñczenie operacji zapisu (Twc=6ms)
    while ( (eeprom_status() & 0x01) ) // Write In Progress (WIP)
        _delay_ms(1);

    return;
}

unsigned char eeprom_read(unsigned long addr)
{
     spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_READ);    // byte read

        // wyœlij adres 24-bitowy
        spi_write( (addr >> 16) & 0xff);
        spi_write( (addr >> 8)  & 0xff);
        spi_write(  addr        & 0xff);

        // odczyt bajt
        unsigned char data = spi_read();

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    return data;
}


void eeprom_page_write(unsigned long addr, unsigned char* buf, unsigned char len)
{
     spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_WRITE);    // byte write

        // wyœlij adres 24-bitowy
        spi_write( (addr >> 16) & 0xff);
        spi_write( (addr >> 8)  & 0xff);
        spi_write(  addr        & 0xff);

        // wyœlij bajty do zapisania
        for (unsigned char i=0; i < len; i++)
            spi_write(buf[i]);

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    //_delay_ms(2);_delay_ms(2);_delay_ms(2);_delay_ms(2); // Twc = 6ms

    // czekaj na zakoñczenie operacji zapisu (Twc=6ms)
    while ( (eeprom_status() & 0x01) ) // Write In Progress (WIP)
        _delay_ms(1);

    return;
}


void eeprom_page_read(unsigned long addr, unsigned char* buf, unsigned char len)
{
     spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_READ);    // byte read

        // wyœlij adres 24-bitowy
        spi_write( (addr >> 16) & 0xff);
        spi_write( (addr >> 8)  & 0xff);
        spi_write(  addr        & 0xff);

        // odczyt bajty
        for (unsigned char i=0; i < len; i++)
            buf[i] = spi_read();

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    return;
}


unsigned char eeprom_read_signature()
{
    spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_RDID);    // release from deep power-down

        // wyœlij adres 24-bitowy (dummy address)
        spi_write('f');
        spi_write('o');
        spi_write('o');

        // odczyta sygnature
        unsigned char sig = spi_read();

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    return sig;
}   


void eeprom_chip_erase()
{
    eeprom_enable_write(1);

    spi_select(EEPROM_SS_PORT, EEPROM_SS_PIN);
    _delay_us(10);
    
        spi_write(EEPROM_CE);    // chip erase

    spi_unselect(EEPROM_SS_PORT, EEPROM_SS_PIN);

    // czekaj na zakoñczenie operacji kasowania
    while ( (eeprom_status() & 0x01) ) // Write In Progress (WIP)
        _delay_ms(1);

    eeprom_enable_write(1);
}

unsigned long eeprom_get_size(void)
{
    unsigned long n = 128; // 128 bajtow = 1kb (25xx010) -> szukamy 131 072 bajtow do 1Mb (25xx1024)

    if (eeprom_read_signature() == 0xff)
        return 0L;

    while (!eeprom_is_size(n) && n < 250000L)
    {
        n *= 2L;
    }

    return n;
}



unsigned char eeprom_is_size(unsigned long n)
{
    unsigned char temp;

    temp = eeprom_read(0L);

    eeprom_enable_write(1);

    if (eeprom_read(n) == temp)
    {
        eeprom_write(0L, temp+1);

        if (eeprom_read(n) == temp+1)
        {
            eeprom_write(0L, temp-1);
            return 1;
        }
    }

    return 0;
}
