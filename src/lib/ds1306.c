#include "ds1306.h"


void ds1306_init(void)
{
    // SS dla RTC jako wyjœcie
    DDR(RTC_SS_PORT) |= (1 << RTC_SS_PIN);
    spi_select(RTC_SS_PORT, RTC_SS_PIN);

    // inicjalizacja rejestru Control Register
    ds1306_write(0x8f, 0x04);   // ustaw bit 1Hz
}

unsigned char ds1306_read(unsigned char addr)
{   
    // maskowanie adresu (A7 == 0 -> odczyt)
    addr &= ~0x80;

    SPCR |= (1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_medium_speed();

    spi_unselect(RTC_SS_PORT, RTC_SS_PIN);  // CE - aktywne stanem wysokim (odwrotnie w stosunku do SS)
    _delay_us(15);

        unsigned char data = spi_write(addr);

        data = spi_read();

    spi_select(RTC_SS_PORT, RTC_SS_PIN);

    SPCR &= ~(1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_full_speed();

    return data;
}

void ds1306_write(unsigned char addr, unsigned char data)
{
    // maskowanie adresu (A7 == 1 -> zapis)
    addr |= 0x80;

    SPCR |= (1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_medium_speed();

    spi_unselect(RTC_SS_PORT, RTC_SS_PIN);  // CE - aktywne stanem wysokim (odwrotnie w stosunku do SS)
    _delay_us(15);

        spi_write(addr);    // control register
        spi_write(data);

    spi_select(RTC_SS_PORT, RTC_SS_PIN);

    SPCR &= ~(1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_full_speed();
}


void ds1306_time_set(time_t *time)
{
    unsigned char buf[7];

    // wype³nij bufor (do wys³ania do RTC)
    buf[0] = num2bcd(time->tm_sec);
    buf[1] = num2bcd(time->tm_min);
    buf[2] = num2bcd(time->tm_hour) & 0x3f;
    buf[3] = num2bcd(time->tm_wday) & 0x07;
    buf[4] = num2bcd(time->tm_mday) & 0x3f;
    buf[5] = num2bcd(time->tm_mon)  & 0x3f;
    buf[6] = num2bcd(time->tm_year) & 0x3f;

    // zapisz czas do rejestrów RTC
    SPCR |= (1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_medium_speed();

    spi_unselect(RTC_SS_PORT, RTC_SS_PIN);  // CE - aktywne stanem wysokim (odwrotnie w stosunku do SS)
    _delay_us(15);

        spi_write(0x80);  // zapisuj w trybie burst od pierwszego (0x00) rejestru

        for (unsigned char i=0; i < 7; i++)
            spi_write(buf[i]);   // zapisuj kolejne rejestry (burst mode)

    spi_select(RTC_SS_PORT, RTC_SS_PIN);

    SPCR &= ~(1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_full_speed();
}

// odczytaj kolejno 7 rejestrów aktualnego czasu (0x00 - 0x06)
void ds1306_time_get(time_t *time)
{
    // pobierz czas z rejestrów RTC
    unsigned char buf[7];

    SPCR |= (1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_medium_speed();

    spi_unselect(RTC_SS_PORT, RTC_SS_PIN);  // CE - aktywne stanem wysokim (odwrotnie w stosunku do SS)
    _delay_us(15);

        spi_write(0x00);  // czytaj w trybie burst od pierwszego (0x00) rejestru

        for (unsigned char i=0; i < 7; i++)
            buf[i] = spi_read();   // czytaj kolejne rejestry (burst mode)

    spi_select(RTC_SS_PORT, RTC_SS_PIN);

    SPCR &= ~(1<<CPHA); // zbocza sygnalu taktujacego CLK
    spi_full_speed();


    // wype³nij podan¹ strukturê time_t
    time->tm_sec  = bcd2num(buf[0]);
    time->tm_min  = bcd2num(buf[1]);
    time->tm_hour = bcd2num(buf[2] & 0x3f);
    time->tm_wday = bcd2num(buf[3] & 0x07);
    time->tm_mday = bcd2num(buf[4] & 0x3f);
    time->tm_mon  = bcd2num(buf[5] & 0x3f);
    time->tm_year = bcd2num(buf[6] & 0x3f);
}
