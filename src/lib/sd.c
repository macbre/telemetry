#include "sd.h"

unsigned char sd_init()
{
    // na razie nie wykryto karty
    sd_state = SD_FAILED;

    // rozmiaru te¿ nie znamy
    sd_size = 0UL;

    // SS dla SD jako wyjœcie
    DDR(SD_SS_PORT) |= (1 << SD_SS_PIN);

    spi_unselect(SD_SS_PORT, SD_SS_PIN);

    // inicjalizacja karty przy taktowaniu SPI ~250 kHz
    spi_low_speed(); 

    unsigned int i, retry = 0;
    unsigned char ret=0;

    // poczekaj na inicjalizacje karty
	for(i=0; i<10; i++){
        spi_write(0xff);
    }

    spi_select(SD_SS_PORT, SD_SS_PIN);

	do  {
		// reset karty, wybor trybu SPI - SD/MMC
		ret = sd_cmd(SD_GO_IDLE_STATE, 0);

		// timeout?
		if( (retry++) > 500 ) {
            spi_unselect(SD_SS_PORT, SD_SS_PIN);
            spi_full_speed(); 
            return 2;
        }

	} while(ret != SD_R1_IDLE_STATE);

    // SEND_OP_COND - SD/MMC
    do {
        ret = sd_cmd(SD_SEND_OP_COND, 0); 

    } while(ret & SD_R1_IDLE_STATE);

    retry = 0;
    
    // spróbuj inicjalizacji kart SD -> APP_CMD i APP_SEND_OP_COND (rozkazy #55 i #41)
    do {
        ret = sd_cmd(SD_APP_CMD, 0);

        ret = sd_cmd(SD_APP_SEND_OP_COND, 0);

        sd_state = SD_IS_SD;

        if ( (retry++) > 32 ) {

            // ok - to karta MMC
            sd_state = SD_IS_MMC;

            ret = 0;
        }

    } while (ret != 0);

	// wylacz sprawdzanie CRC dla uproszczenia transmisji
	ret = sd_cmd(SD_CRC_ON_OFF, 0);

	// ustaw rozmiar bloku na 512 bajtow
	ret = sd_cmd(SD_SET_BLOCKLEN, SD_BLOCK_SIZE);

    // koniec inicjalizacja (odznacz karte, przywroc taktowanie SPI)
    spi_unselect(SD_SS_PORT, SD_SS_PIN);
    spi_full_speed(); 

	// ok!
	return 1;
}


unsigned char sd_info(sd_cardid* info)
{
	unsigned char i, retry, data;
	
	spi_select(SD_SS_PORT, SD_SS_PIN);
	
    // przygotuj pamiec struktury sd_cardid
    memset((void*) info, 0, sizeof(sd_cardid));

    //
    // CID
    //

    data = sd_cmd(SD_SEND_CID, 0);

    // czekaj na odpowiedz
    retry = 0;

    while (data != SD_STARTBLOCK_READ) {

        data = spi_read();

        //rs_hex(data); rs_send('a');

		if (retry++ > 200) {
            spi_unselect(SD_SS_PORT, SD_SS_PIN);    // !!!
            return 2;
        }
    }

   
    // wpisuj dane do struktury
	for(i = 0; i < 16; i++) {
		*( (unsigned char*) info + i ) = spi_read();
    }

    // odczyt tag koñca danych
    spi_read();
    spi_read();

    //rs_dump(ptr, 16);

    // korekta zawartosci pol
    info->mdt = HTONS(info->mdt);
    info->crc = info->crc >> 1;

    //
    // CSD
    //

    // pojemnosc karty (w bajtach)
    unsigned char multiplier;
    unsigned char blocksize; // zwykle 512 bajtow -> 9
    unsigned char buf[18];

    // pobierz strukture CSD i oblicz pojemnosc karty
    //
    // @see MAXQ2000 Secure Digital (SD) Card Interface via SPI
    data = sd_cmd(SD_SEND_CSD, 0);

    // czekaj na odpowiedz
    retry = 0;

    while (data != SD_STARTBLOCK_READ) {

        data = spi_read();

        //rs_hex(data); rs_send('b');

    	if (retry++ > 200) {
            spi_unselect(SD_SS_PORT, SD_SS_PIN);    // !!!
            return 3;
        }
    }



    // zapisz dane do bufora
    for (i=0; i<18; i++) {
        buf[i] = spi_read();
    }
    
    //rs_dump(buf, 18);

    // rozmiar bloku (2^blocksize bajtow)
    blocksize = buf[5] & 0x0f;

    // pojemnosc karty
    info->capacity = ((buf[6] & 0x03) << 10) | (buf[7] << 2) | (buf[8] & 0xc0);

    // mnoznik pojemnosci karty
    multiplier = ((buf[9] & 0x03) << 1) | (buf[10] >> 7);

    // oblicz pojemnosc karty
    info->capacity <<= (multiplier + blocksize + 2);

    // przepisz rozmiar karty
    sd_size = info->capacity;

    spi_unselect(SD_SS_PORT, SD_SS_PIN);

    return 1;
}


unsigned char sd_get_state()
{
    return sd_state;
}


unsigned char sd_command(unsigned char cmd, unsigned long arg)
{
	// CS karty w stan niski
	spi_select(SD_SS_PORT, SD_SS_PIN);

	// wyslij rozkaz
	unsigned char ret = sd_cmd(cmd, arg);
	
    // CS karty w stan wysoki
	spi_unselect(SD_SS_PORT, SD_SS_PIN);

	return ret;
}

unsigned char sd_cmd(unsigned char cmd, unsigned long arg)
{
    unsigned char ret, retry=0;

	// wyslij komende
    spi_write( (cmd & 0x7F) | 0x40 );

    // argument
	spi_write(arg>>24);
	spi_write(arg>>16);
	spi_write(arg>>8);
	spi_write(arg);

    // CRC zgodne tylko dla MMC_GO_IDLE_STATE (dla reszty komend sprawdzanie CRC jest wylaczone)
	spi_write( (cmd == SD_GO_IDLE_STATE) ?  0x95 : 0xff);

    // debug
    //rs_newline(); rs_hex(cmd); rs_send('|'); rs_hex(crc); rs_newline();
	
	// czekaj na odpowiedz
	while( (ret = spi_read()) == 0xFF)
		if (retry++ > 8) break;
	
    // dodatkowe takty zegara SPI dla instrukcji inicjalizacyjnych (nie wysy³aj wykrytym kartom MMC!)
    if (sd_state != SD_IS_MMC)
        spi_write(0xFF);

    // zwroc odpowiedz
	return ret;
}


// @see MAXIM AN3969
/*
unsigned char sd_crc7(unsigned char old_crc, unsigned char data)
{
    unsigned char new_crc;
    signed char x;
    
    new_crc = old_crc;

    for (x = 7; x >= 0; x--) {
        new_crc <<= 1;
        new_crc |= SD_GETBIT(data,x);

        if (SD_GETBIT(new_crc, 7) == 1) {
            new_crc ^= 0x89; 
        }
    }

    return new_crc; 
}

// @see MAXIM AN3969
unsigned char sd_crc7_finalize(unsigned char old_crc)
{
    unsigned char new_crc, x;

    new_crc = old_crc;

    for (x = 0; x < 7; x++) {
        new_crc <<= 1;
        if (SD_GETBIT(new_crc, 7) == 1) {
            new_crc ^= 0x89;
        }
    }

    return new_crc;
} 
*/

unsigned char sd_read_block(unsigned long sector, unsigned char* buffer)
{
	unsigned char ret;
	unsigned int i;

    // przygotuj bufor
    memset((void*) buffer, 0, SD_BLOCK_SIZE);

	// CS karty w stan niski
	spi_select(SD_SS_PORT, SD_SS_PIN);

	// wyslij komende odczytu bloku (2^9 = 512 bajtow)
    ret = sd_cmd(SD_READ_SINGLE_BLOCK, sector<<9);
	
	// sprawdz odpowiedz
	if(ret != 0x00) {
        spi_unselect(SD_SS_PORT, SD_SS_PIN);
		return 0;
    }

	// czekaj na token poczatku danych
    while(spi_read() != SD_STARTBLOCK_READ);

	// pobierz dane
	for(i=0; i<SD_BLOCK_SIZE; i++) {
		buffer[i] = spi_read();
	}
    
    // CRC
    spi_read();
    spi_read();

	// CS karty w stan wysoki
	spi_unselect(SD_SS_PORT, SD_SS_PIN);

    //rs_dump(buffer, SD_BLOCK_SIZE);
	
	return 1;
}

unsigned char sd_write_block(unsigned long sector, unsigned char* buffer)
{
	unsigned char ret;
	unsigned int i;

	// CS karty w stan niski
	spi_select(SD_SS_PORT, SD_SS_PIN);

	// wyslij komende zapisu bloku (2^9 = 512 bajtow)
	ret = sd_cmd(SD_WRITE_BLOCK, sector<<9);

	// sprawdz odpowiedz
	if(ret != 0x00) {
        spi_unselect(SD_SS_PORT, SD_SS_PIN);
		return 0;
    }

	//spi_write(0xFF);

	// token poczatku danych
	spi_write(SD_STARTBLOCK_WRITE);
	
    // przeslij kolejne bajty z bufora
	for(i=0; i<SD_BLOCK_SIZE; i++) {
		spi_write(buffer[i]);
	}

	// wyslij CRC (wylaczylismy jego sprawdzanie przy inicjalizacji)
	spi_write(0xff);
    spi_write(0xff);

	// token odpowiedzi (czy karta przyjela dane?)
	ret = spi_read();

	if( (ret & SD_DR_MASK) != SD_DR_ACCEPT) {
        spi_unselect(SD_SS_PORT, SD_SS_PIN);
		return 0;
    }

	// poczekaj na zwolnienie karty (w czasie operacji zapisu na linii MISO stan niski)
	while(!spi_read());
	
    // CS karty w stan wysoki
	spi_unselect(SD_SS_PORT, SD_SS_PIN);

    //rs_dump(buffer, SD_BLOCK_SIZE);

	return 1;
}
/**

unsigned char sd_read_stream(unsigned long addr, unsigned char* buffer, unsigned int len) {

    unsigned char ret;
	unsigned int i;

    // CS karty w stan niski
	spi_select(SD_SS_PORT, SD_SS_PIN);

	// wyslij komende odczytu sekwencyjnego
	ret = sd_cmd(SD_READ_STREAM, addr); rs_hex(ret);

	// sprawdz odpowiedz
	if(ret != 0x00) {
        spi_unselect(SD_SS_PORT, SD_SS_PIN);
		return 0;
    }

	// czekaj na token poczatku danych
	//while(spi_read() != SD_STARTBLOCK_READ);

	// pobierz dane
	for(i=0; i<len; i++) {
		*buffer++ = spi_read();
	}

	// zakoncz transfer
    ret = sd_cmd(SD_STREAM_STOP, 0UL); rs_hex(ret);

	// CS karty w stan wysoki
	spi_unselect(SD_SS_PORT, SD_SS_PIN);
	
	return 1;
}


unsigned char sd_write_stream(unsigned long addr, unsigned char* buffer, unsigned int len) {
    return 1;
}
**/
