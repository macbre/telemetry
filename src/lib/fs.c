#include "fs.h"

unsigned char fs_init(unsigned char* buf) {
    // ustawienie wskazanego bufora na operacje I/O
    fs_buf = buf;
    return fs_is_medium_ok();
}

unsigned char fs_open(fs_file* file, unsigned char* name, unsigned char params) {
    
    // numer sektora
    unsigned long sector;

    // dane sektora
    fs_sector* sector_data = (fs_sector*) fs_buf;

    // inicjalizacja struktury file
    memset((void*) file, 0, sizeof(fs_file));

    // zakoñcz nazwê znakiem NULL - ogranicz d³ugie ³añcuchy
    name[8] = 0;

    // skopiuj nazwê pliku do struktury file
    strcpy((char*)file->name, (char*)name);

    // szukaj podanego pliku w systemie
    sector = fs_find(name);

    // znaleziono plik
    if (sector) {

        //rs_dump((void*)fs_buf, 512);

        file->timestamp = sector_data->timestamp;
        file->sector    = sector;
        file->size      = sector_data->size;
        file->pos       = 0;

        return 1;
    }

    // nie twórz nowego pliku
    if (params & FS_DONT_CREATE) {
        return 0;
    }

    //
    // utwórz nowy plik
    //

    // szukaj miejsca
    sector = fs_find_sector();

    // jest miejsce
    if (sector) {

        file->sector = sector;
        file->size   = 0;
        file->pos    = 0;

        // zapisz sektor na kartê
        memset((void*) sector_data, 0, 512);

        // ustaw dane
        sector_data->flag = FS_FLAG_FILE;
        strcpy((char*)(sector_data->name), (char*)file->name);
        sector_data->timestamp = 0UL;
        sector_data->size = 0;

        // zapis
        fs_write_sector(sector);
        //rs_dump((void*)fs_buf, 512);

        return 1;
    }

    // nie ma miejsca
    return 0;
}

// dopisuje dane z buf (o d³ugoœci len) do koñca pliku file
unsigned int fs_write(fs_file* file, unsigned char* buf, unsigned int len) {

    // kontrola d³ugoœci pliku
    if (FS_MAX_FILE_SIZE < (file->size + len) || !len ) {
        return 0;
    }

    // dane sektora
    fs_sector* sector_data = (fs_sector*) fs_buf;

    // odczytaj sektor
    fs_read_sector(file->sector);

    // kopiuj dane na koniec pliku
    memcpy( (void*)(sector_data->data) + sector_data->size, (void*)buf, len);

    // zwiêksz informacjê o rozmiarze pliku
    sector_data->size += len;
    file->size += len;

    // zapisz na kartê
    fs_write_sector(file->sector);

    return len;
}

// oczytuje dane do buf (o d³ugoœci len) z pliku file z pozycji file->pos
unsigned int fs_read(fs_file* file, unsigned char* buf, unsigned int len) {

    // kontrola po³o¿enia wskaŸnika pliku
    if (file->size < (file->pos + len) || !len ) {
        return 0;
    }

    // dane sektora
    fs_sector* sector_data = (fs_sector*) fs_buf;

    // odczytaj sektor
    fs_read_sector(file->sector);

    // kopiuj dane
    memcpy((void*)buf, (void*)(sector_data->data) + file->pos, len);

    // zwiêksz informacjê o wskaŸniku pliku
    file->pos += len;

    return len;
}

// zamknij plik
unsigned char fs_close(fs_file* file) {
    return 1;
}

// skasuj plik
unsigned char fs_delete(fs_file* file) {

    // zamazanie zawartoœci pliku
    memset((void*) fs_buf, 0xff, 512);

    // zapis
    fs_write_sector(file->sector);

    return 1;
}

// szuka podanego pliku i zwraca numer zajmowanego przez niego sektora
unsigned long fs_find(unsigned char* name) {
    unsigned long sector;
    unsigned long sectors = fs_number_of_sectors();

    // zrzutuj dane sektora do struktury jego opisu
    fs_sector* sector_data = (fs_sector*) fs_buf;
    
    // sprawdzaj kolejne sektory
    for (sector = 1; sector < sectors; sector++) {
        fs_read_sector(sector);

        // szukaj tylko wœród sektorów oznaczonych jako zawieraj¹ce pliki
        if ( sector_data->flag != FS_FLAG_FILE )
            continue;

        // znaleziono plik?
        if ( strcmp((char*)(sector_data->name), (char*)name) == 0 )
            return sector;
    }
    
    return 0;
}

// szukaj pierwszego wolnego sektora
unsigned long fs_find_sector() {
    
    unsigned long sector;
    unsigned long sectors = fs_number_of_sectors();

    // zrzutuj dane sektora do struktury jego opisu
    fs_sector* sector_data = (fs_sector*) fs_buf;
    
    // sprawdzaj kolejne sektory
    for (sector = 1; sector < sectors; sector++) {
        fs_read_sector(sector);

        // znaleziono pusty sektor - nieustawiona flaga FS_FILE
        if ( sector_data->flag != FS_FLAG_FILE )
            return sector;
    }
    
    return 0;
}

// listuje pliki szukaj¹c w systemie pocz¹wszy od podanego sektora
unsigned long fs_list_files(fs_file* file, unsigned long sector) {
    
    // liczba sektorów dostêpnych dla systemu plików
    unsigned long sectors = fs_number_of_sectors();
    
    // zrzutuj dane sektora do struktury jego opisu
    fs_sector* sector_data = (fs_sector*) fs_buf;

    // za ka¿dym razem szukaj kolejnego sektora
    for (; sector < sectors; sector++) {
        fs_read_sector(sector);

        // znaleziono sektor z plikiem
        if ( sector_data->flag == FS_FLAG_FILE ) {
            // kopiuj nazwê pliku
            strcpy( (char*)file->name, (char*)(sector_data->name));

            //rs_send('>'); rs_text((char*)file->name); rs_newline();

            // pozosta³e dane o pliku
            file->timestamp = sector_data->timestamp;
            file->sector    = sector;
            file->size      = sector_data->size;
            file->pos       = 0;

            return ++sector;
        }
    }


    return 0;
}
