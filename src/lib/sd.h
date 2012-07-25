#ifndef _SD_H_
#define	_SD_H_

#include "../telemetry.h"

//#define SD_SS_PORT     PORTB
//#define SD_SS_PIN      1

// @see http://kju.wemif.net/mmc/?p=3

//
// komendy karty SD/MMC
//

// podstawowe 
#define SD_GO_IDLE_STATE			0		// inicjalizacja karty (wybor trybu SPI)
#define SD_SEND_OP_COND			    1		// ustawienie trybu pracy
#define SD_ALL_SEND_CID             2       // wszystkie karty wysy³aj¹ swoje CID
#define SD_SEND_RELATIVE_ADDR       3       // przypisz adres do karty
#define SD_SEND_CSD				    9		// pobranie CSD (parametry pracy karty)
#define SD_SEND_CID				    10		// pobranie CID (dane producenta karty)

// odczyt strumieniowy
#define SD_READ_STREAM				11      // odczyt "strumieniowy" kolejnych bajtow od podanego adresu
#define SD_STREAM_STOP 		        12      // zatrzymaj transmisje "strumieniowa"
#define SD_SEND_STATUS				13      // pobierz status karty

// odczyt blokowy
#define SD_SET_BLOCKLEN			    16		// wybor liczby bajtow na blok
#define SD_READ_SINGLE_BLOCK		17		// odczyt bloku
#define SD_READ_MULTIPLE_BLOCKS		18		// odczyt kolejnych blokow

// zapis strumieniowy
#define SD_WRITE_STREAM		        20		// zapis "strumieniowy" kolejnych bajtow od podanego adresu

// zapis blokowy
#define SD_WRITE_BLOCK				24		// zapis bloku

// blokady zapisuj
#define SD_SET_WRITE_PROT			28
#define SD_CLR_WRITE_PROT			29
#define SD_SEND_WRITE_PROT			30

// oznaczeanie blokow do kasowania
#define SD_TAG_SECTOR_START		    32
#define SD_TAG_SECTOR_END			33
#define SD_UNTAG_SECTOR			    34
#define SD_TAG_ERASE_GROUP_START 	35		// czyszczenie karty ...
#define SD_TAG_ERARE_GROUP_END		36		// ...
#define SD_UNTAG_ERASE_GROUP		37		// ...
#define SD_ERASE					38		// czyszczenie karty (wykonaj)
#define SD_APP_CMD                  55      // na komende odpowie karta SD, karta MMC - nie!
#define SD_CRC_ON_OFF				59		// wlaczenie/wylaczenie sprawdzania CRC

// komendy poprzedzone rozkazem SD_APP_CMD (tylko karty SD!)
#define SD_APP_SD_STATUS            0x0d
#define SD_APP_SEND_NUM_WR_BLOCKS   0x16
#define SD_APP_SET_WR_BLK_ERASE_CNT 0x17
#define SD_APP_SEND_OP_COND         0x29    // ACMD41
#define SD_APP_SET_CLR_CARD_DETECT  0x2a
#define SD_APP_SEND_SCR             0x33 

// odpowiedzi R1
#define SD_R1_BUSY					0x80	// karta "zajeta"
#define SD_R1_PARAMETER			    0x40
#define SD_R1_ADDRESS				0x20
#define SD_R1_ERASE_SEQ			    0x10
#define SD_R1_COM_CRC				0x08
#define SD_R1_ILLEGAL_COM			0x04
#define SD_R1_ERASE_RESET			0x02
#define SD_R1_IDLE_STATE			0x01

// odpowiedzi R2
#define SD_R2_BEGIN                 0x3F

// tokeny pocz¹tku bloku danych
#define SD_STARTBLOCK_READ			0xFE	// po tym tokenie karta wysle dane z bloku
#define SD_STARTBLOCK_WRITE		    0xFE	// po tym tokenie wysylamy do karty dane do bloku
#define SD_STARTBLOCK_MWRITE		0xFC


// koniec danych
#define SD_STOPTRAN_WRITE			0xFD

// blad danych
#define SD_DE_MASK					0x1F
#define SD_DE_ERROR				    0x01
#define SD_DE_CC_ERROR				0x02
#define SD_DE_ECC_FAIL				0x04
#define SD_DE_OUT_OF_RANGE			0x04
#define SD_DE_CARD_LOCKED			0x04

// odpowiedz danych
#define SD_DR_MASK					0x1F
#define SD_DR_ACCEPT				0x05
#define SD_DR_REJECT_CRC			0x0B
#define SD_DR_REJECT_WRITE_ERROR	0x0D

// rozmiar bloku przesylanych danych
#define SD_BLOCK_SIZE               512

// bufor na blok danych z/do karty
//unsigned char sd_buffer[SD_BLOCK_SIZE];

// informacja o karcie
typedef struct
{
    /* CID */
	unsigned char mid;      // Manufacturer ID
	unsigned int  oid;      // OEM/Application ID
	unsigned char name[5];  // Product name
	unsigned char revision; // Product revision
    unsigned char psn[4];   // Product Serial Number
	unsigned int  mdt;      // Manufacturing date (BCD) => 0x014 (20yy-m dla SD / m-1997+yy dla MMC)
	unsigned char crc;      // CRC

    /* CSD (fragmenty) */
    unsigned long capacity;  // pojemnosc karty (w bajtach)

} sd_cardid; /* 16 (CID) + 4 (long) */

// stan karty SD/MMC
unsigned char sd_state;

// rozmiar karty (bajty)
unsigned long sd_size;

#define SD_FAILED   0
#define SD_IS_SD    1
#define SD_IS_MMC   2

// inicjalizacja karty
unsigned char sd_init();

// odczyt informacji o karcie
unsigned char sd_info(sd_cardid*);

// czy karta jest gotowa? MMC czy SD?
unsigned char sd_get_state();

// wyslij komende do karty (wraz z ustawieniem CS karty)
unsigned char sd_command(unsigned char, unsigned long);

// wyslij komende do karty (funkcja wewnetrzna biblioteki)
unsigned char sd_cmd(unsigned char, unsigned long);

// odczyt/zapis blokow karty
unsigned char sd_read_block(unsigned long, unsigned char*);
unsigned char sd_write_block(unsigned long, unsigned char*);

// odczyt/zapis strumieniowy
//unsigned char sd_read_stream(unsigned long, unsigned char*, unsigned int);
//unsigned char sd_write_stream(unsigned long, unsigned char*, unsigned int);

// obs³uga sum kontrolnych CRC
/*
#define SD_GETBIT(in, bit) ((in & (1<<bit)) >> bit) 

unsigned char sd_crc7(unsigned char, unsigned char);
unsigned char sd_crc7_finalize(unsigned char);
*/

#endif
