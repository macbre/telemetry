#ifndef _FS_H
#define _FS_H

#define FS_SECTOR_END   0xffffff00

#define FS_FLAG_EMPTY   0x00
#define FS_FLAG_FILE    0x01
#define FS_FLAG_END     0x0f

#define FS_DONT_CREATE    1

// pojedynczy sektor zapisany na karcie
typedef struct {
    unsigned char flag;         // flaga typu sektorów
    unsigned char name[9];      // nazwa pliku
    unsigned long timestamp;    // czas zapisania pliku
    unsigned long size;         // rozmiar pliku zapisanego w sektorze
    unsigned char data[];       // dane pliku
} fs_sector; // 1 + 9 + 4 + 4 + 494 = 512

// maksymalny rozmiar pliku
#define FS_MAX_FILE_SIZE 494

// struktura opisu pliku
typedef struct {
    unsigned char name[9];
    unsigned long timestamp;
    unsigned long sector;
    unsigned long size;
    unsigned long pos;
} fs_file;

#include "../telemetry.h"

// bufor na operacje I/O
unsigned char* fs_buf;

// inicjalizacja systemu plików
unsigned char fs_init(unsigned char*);

// otwórz plik
unsigned char fs_open(fs_file*, unsigned char*, unsigned char);

// zapisz do / odczytaj z pliku (zwracaj¹ liczbê zapisanych / odczytanych bajtów)
unsigned int fs_write(fs_file*, unsigned char*, unsigned int);
unsigned int fs_read(fs_file*, unsigned char*, unsigned int);

// zamknij plik - aktualizuj dane o pliku
unsigned char fs_close(fs_file*);

// usuñ plik
unsigned char fs_delete(fs_file*);

// znajdŸ podany plik
unsigned long fs_find(unsigned char*);

// utwórz plik i zaalokuj wstêpnie jeden sektor
unsigned char fs_create(unsigned char*);

// znajdŸ pierwszy wolny sektor (zwraca FS_SECTOR_END, gdy brak miejsca)
unsigned long fs_find_sector();

// listuje pliki szukaj¹c w systemie pocz¹wszy od podanego sektora
unsigned long fs_list_files(fs_file*, unsigned long);

// funkcje zapisu / odczytu sektorów urz¹dzenia (warstwa abstrakcji)
#define fs_read_sector(sector)    sd_read_block(sector, fs_buf)
#define fs_write_sector(sector)   sd_write_block(sector, fs_buf)

// sprawdŸ urz¹dzenie
#define fs_is_medium_ok()         ( sd_get_state() != SD_FAILED )

// zwraca liczbê sektorów (512 bajtów) urz¹dzenia
//#define fs_number_of_sectors()   (sd_size >> 9)  // / 512
#define fs_number_of_sectors()   128 // ograniczenie

#endif
