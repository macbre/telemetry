#include "firmware.h"

unsigned int firmware_handle_packet(unsigned char* data)
{
    firmware_packet *packet = (firmware_packet*) data;

    rs_text_P(PSTR("Firmware: offset "));
    rs_hex( packet->offset >> 16);
    rs_hex( packet->offset >> 8);
    rs_hex( packet->offset);

    rs_send(' '); rs_send('#'); rs_hex(packet->op);

    switch (packet->op) {
    
        // pakiet z danymi strony do wgrania do pamieci
        case FIRMWARE_OP_SEND:
            
            // na poczatku aktualizacji wyczysc cala pamiec
            if (packet->offset == 0L) {
                eeprom_enable_write(1);
                eeprom_chip_erase();
            }

            // wlacz zapis do pamieci EEPROM
            eeprom_enable_write(1);

            // zapisz dane do pamieci
            eeprom_page_write(packet->offset, packet->data, packet->len);

            //rs_dump(packet->data, packet->len);

            // odsylamy pakiet z potwierdzeniem wgrania danych
            packet->op = FIRMWARE_OP_SEND_OK;

            return 6 + packet->len;

        // weryfikacja danych (odeslij zawartosci podanej strony)
        case FIRMWARE_OP_VERIFY:
            
            // pobierz cala strone
            packet->len = 255;

            // pobierz dane
            eeprom_page_read(packet->offset, packet->data, packet->len);

            //rs_dump(packet->data, packet->len+1);

            // odsylamy pakiet z danymi z pamieci (skrypt aktualizujacy dokona porownania)
            return 6 + packet->len + 1;

        // pakiet z blednym kodem operacji
        default:
            packet->op     = 0;
            packet->offset = 0UL;
            packet->len    = 0;
            return 6;
    }

    return 0;
}

unsigned int firmware_get_sector_length(unsigned char sector)
{
    unsigned int length;

    // little endian...
    length  = ( (unsigned int) eeprom_read(2L*sector + 1) ) << 8;
    length += eeprom_read(2L*sector);

    return length;
}

unsigned int firmware_read_sector(unsigned char sector, unsigned char* buf)
{
    unsigned int length, len;

    rs_text_P(PSTR("Firmware: czytam sektor #")); rs_hex(sector); rs_send(' ');

    // pobierz rozmiar danych w sektorze
    length = firmware_get_sector_length(sector);

    rs_int(length); rs_send('B'); rs_newline();

    // kontrola d³ugoœci pakietu
    if (length > FIRMWARE_SECTOR_SIZE)
        return 0;

    // pierwsza strona w danym sektorze
    unsigned int page = sector * FIRMWARE_SECTOR_SIZE;

    len = length;

    while (len > 0) {
        // czytaj po stronie pamieci (po 256 bajtow)
        eeprom_page_read(page, buf, (len > 255 ? 255 : len));

        //rs_dump(buf, 255);

        // przesun wskaznik docelowy dla danych
        buf += 255;
        len -= (len > 255 ? 255 : len);
        page += 0x0100;
    }

    //rs_text_P(PSTR("Returned -> ")); rs_int(length); rs_send('B'); rs_newline();

    return length;
}
