#ifndef _TELEMETRY_H
#define _TELEMETRY_H

//
// START:       konfiguracja systemu
//

// przesuniecie czasu wzgledem UTC
//#define UTC_OFFSET      -1    // czas zimowy
#define UTC_OFFSET      -2      // czas letni

// konfiguracja taktowania uC
#define F_CPU 16000000UL      // 16MHz

// definicje pinow Chip Select ukladow podpietych pod SPI
//
// DS1306
#define RTC_SS_PORT     PORTB
#define RTC_SS_PIN      0

// karta SD/MMC
#define SD_SS_PORT      PORTB
#define SD_SS_PIN       1

// 25LC1024
#define EEPROM_SS_PORT  PORTB
#define EEPROM_SS_PIN   2

// ENC28J60
#define ENC28_SS_PORT   PORTB
#define ENC28_SS_PIN    3

// sterownik PWM
//
#define PWM_PORT        PORTA
#define PWM_MASK        0xff

// sterownik PID
#define PID_COUNT       5

// ds18b20
//
#define DS_DEVICES_MAX  8

// ustawienie pinu uC do obslugi 1wire
#define OW_PORT         PORTC
#define OW_PIN          7

// piny dla przycisków
#define KEYS_S1_PORT PORTD
#define KEYS_S1_BIT  6 // 4
#define KEYS_S1      1

#define KEYS_S2_PORT PORTD
#define KEYS_S2_BIT  5
#define KEYS_S2      2

#define KEYS_S3_PORT PORTD
#define KEYS_S3_BIT  4 // 6
#define KEYS_S3      4

#define KEYS_S4_PORT PORTD
#define KEYS_S4_BIT  7
#define KEYS_S4      8

// warstwa sieciowa (domyœlne wartoœci)
#define NET_MAC     {0x00, 0x05, 0x4F, 0x50, 0x55, 0x54}        // MAC: 00-05-4F P-U-T (private)
#define NET_IP      {192, 168, 1, 12}                           // IP: 192.168.1.12 (domyœlny przy braku DHCP w sieci)

//
// KONIEC:      konfiguracja systemu
//


// przydatne makra
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~(1<<bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= (1<<bit))

#define DDR(x) (_SFR_IO8(_SFR_IO_ADDR(x)-1))
#define PIN(x) (_SFR_IO8(_SFR_IO_ADDR(x)-2))

#define nop() __asm__ __volatile__ ("nop")

#define bcd2num(val)     (((val) & 0x0f) + ((val)>>4)*10)
#define num2bcd(val)     ((((val)/10)<<4) + (val)%10)

#define dec2hex(val)  ( (val) < 10 ? '0' + (val) : 'A'-10 + (val) )

// zmienne globalne
volatile unsigned char pooling_step;  // krok poolingu
volatile unsigned long uptime;        // uptime systemu w sekundach

// konfiguracja I/O wybranego uC
#include <avr/io.h>

// opóŸnienia
#include <util/delay.h>

// abs() / random() / itoa()
#include <stdlib.h>

// memset
#include <string.h>

// obsluga stalych w pamieci programu (FLASH)
#include <avr/pgmspace.h>

// watchdog
#include <avr/wdt.h>

// reset AVR'a
#define soft_reset()        \
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));


// przerwania
#include <avr/interrupt.h>

// EEPROM uC
#include <avr/eeprom.h>

// struktura czasu
typedef struct
{
    unsigned char tm_sec;        // sekundy   (0-59)
    unsigned char tm_min;        // minuty    (0-59)
    unsigned char tm_hour;       // godziny   (0-23)
    unsigned char tm_wday;       // dzieñ tyg (1-7) / (niedziela-sobota)
    unsigned char tm_mday;       // dzieñ mie (1-31)
    unsigned char tm_mon;        // miesi¹c   (1-12)
    unsigned char tm_year;       // rok       (0-99)
} time_t;

//
// dolacz zewnetrzne biblioteki
//

// RS232
#include "lib/rs.h"

// wyswietlacz 2x16
#define LCD_USE_POLISH_CHARS // uzywaj polskich znakow

#include "lib/lcd.h"


// klawisze
#include "lib/keys.h"

// menu
#include "lib/menu.h"

// SPI
#include "lib/spi.h"    // g³ówna biblioteka obs³ugi SPI

#include "lib/ds1306.h" // uk³ad RTC DS1306
#include "lib/eeprom.h" // pamieci EEPROM (zgodne z AT25*)
#include "lib/sd.h"     // karta SD/MMC
#include "lib/fs.h"     // FS: bardzo prosty system plików
#include "lib/enc28.h"  // kontroler Ethernetu ENC28J60

// 1wire
#include "lib/1wire.h"
#include "lib/ds18b20.h" // czujnik temperatury

// "stos" TCP/IP
#include "lib/net.h"      // struktury pakietów w sieci Internet
#include "lib/webpage.h"  // obsluga zadan HTTP
#include "lib/firmware.h" // aktualizacja / pobieranie danych z firmware'u (pamiec EEPROM)
#include "lib/daq.h"      // obsluga ¿¹dañ DAQ na porcie UDP (MATLAB)

// sterownik PWM PID
#include "lib/pwm.h"    // programowy, oœmiokana³owy sterownik PWM
//#include "lib/pid.h"    // regulator PID

// ustawienia systemu przechowywane w pamiêci EEPROM uC
#include "lib/config.h"

// przerwanie INT1
void on_int1();

// komenda RS
void on_rs_cmd(unsigned char);



//
// ZMIENNE GLOBALNE
//

// identyfikacja programu (wartosci zdefiniowane w telemetry.c)
extern const char PROGRAM_NAME[] PROGMEM;
extern const char PROGRAM_COPYRIGHT[] PROGMEM;
extern const char PROGRAM_VERSION1[] PROGMEM;
extern const char PROGRAM_VERSION2[] PROGMEM;

// regulatory PID
//pid_regulator pid[PID_COUNT];

// partycja FAT
//fat_partition fat;


#endif
