// @see http://en.wikipedia.org/wiki/File_Allocation_Table
// @see Microsoft fatgen103.doc

#ifndef _FAT_H
#define _FAT_H

#include "../telemetry.h"

// sta³e predefiniowane
#define FAT_PART_TYPE			0x1C2	// bajt sektora MBR, w którym zapisany typ partycji
#define FAT_PART_FIRST_SECTOR	0x1C6	// bajt sektora MBR, w którym zapisany numer pierwszego sektora pierwszej partycji

// typy partycji FAT
#define FAT_FAT12   	0x01
#define FAT_FAT16	    0x06
#define FAT_FAT32	    0x0B
#define FAT_UNKNOWN     0x00

// atrybuty plików
#define FAT_ATTR_READ_ONLY	0x01
#define FAT_ATTR_HIDDEN		0x02
#define FAT_ATTR_SYSTEM		0x04
#define FAT_ATTR_ARCHIVE    0x20

// funkcje zapisu / odczytu sektorów urz¹dzenia (warstwa abstrakcji)
#define FAT_READ_SECTOR(sector, buf)    sd_read_block(sector, buf)
#define FAT_WRITE_SECTOR(sector, buf)   sd_write_block(sector, buf)

// struktury
typedef struct 
{
	unsigned long part_first_sector_off; 	
	unsigned char type;						
    unsigned char name[6];
	
	//Partition Boot Record:					offset:		Name:									Size:
	unsigned int 	reserved_sectors;		///< 0x0E		Zarezerwowane Sektory po Boot Record	2 bytes	
	unsigned int 	bytes_per_sector;		///< 0x0B		Bajtow na sektor						2 bytes
	unsigned char 	sectors_per_cluster;	///< 0x0D		sektorow na klaster						1 byte
	unsigned char 	number_of_fat_tables;	///< 0x10		Ilosc kopii tablic FAT (zwykle 2)		1 byte
	unsigned int 	max_root_dir_entries;	///< 0x11		Max liczba wpisow w Root Directory		2 bytes
	unsigned int 	sectors_per_fat;		///< 0x16		Liczba sektorow na jedna tablice FAT	2 bytes
	unsigned int 	first_cluster_off ;		///< Pierwszy klaster danych
	unsigned int  	root_dir_off;			///< Pierwszy sektor Root Directory
	unsigned int  	fat_off;				///< Pierwszy sektor tablicy FAT
	unsigned int 	total_sectors;			///< 0x13		Calkowita liczba dostenych sektorow		2 bytes

} fat_partition;

typedef struct 
{
	unsigned char name[8];				// nazwa pliku
	unsigned char ext[3];				// rozszerzenie
	unsigned long size;					// rozmiar pliku
	unsigned int first_cluster;			// pierwszy klaster pliku
	unsigned int number_of_entry_in_dir;// numer wpisu w directory table
	unsigned int last_cluster; 			// ostatni kalster
	unsigned long cur_pos;				// aktualnie obslugiwany bajt pliku
} fat_file; 


// wskaŸniki na bufory - ustawiane w fat_init()
unsigned char* fat_buffer;
fat_partition* fat_struct;

// inicjalizacja systemu - odczyt danych o partycji, wykrycie typu, rozmiaru
unsigned char fat_init(fat_partition*, unsigned char*);

// odczyt / zapis wartoœci klastra z tablicy alokacji
unsigned int fat_cluster_read(unsigned long);
void fat_cluster_write(unsigned long, unsigned int);


//
// obs³uga plików
//

// otwórz istniej¹cy / utwórz nowy plik
unsigned char fat_file_open(fat_file*, unsigned char*, unsigned char*);

// zapis do pliku
unsigned char fat_file_write(fat_file*, unsigned char*, unsigned int);

// odczyt z pliku
unsigned char fat_file_read(fat_file*, unsigned char*, unsigned int);

// zamknij plik
unsigned char fat_file_close(fat_file*);


//
// funkcje wewnêtrzne
//

// walidacja znaku jako zgodnego z FAT
unsigned char fat_validate_char(unsigned char);

// porównaj pliki
unsigned char fat_check_root_dir_entry(fat_file*, unsigned char*);

// znajduje numer pierwszego wolnego klustra
unsigned long fat_find_free_cluster(void);

// zwraca aktualny czas w formacie FAT'owskim
//unsigned int fat_get_time();

#endif
