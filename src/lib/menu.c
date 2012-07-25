#include "menu.h"

void menu_init() {
    menu_pos = 0;
    menu_sub_pos = 0;
    menu_updated = 1;
}

void menu_handle_keys() {
    
    // górna para lewo/prawo
    if ( keys_get(KEYS_S1) ) {
        menu_pos--;

        menu_sub_pos = 0;
        menu_updated = 1;
    }
    else if ( keys_get(KEYS_S2) ) {
        menu_pos++;

        menu_sub_pos = 0;
        menu_updated = 1;
    }
    
    // dolna para lewo/prawo
    if ( keys_get(KEYS_S3) ) {
        menu_sub_pos--;

        menu_updated = 1;
    }
    else if ( keys_get(KEYS_S4) ) {
        menu_sub_pos++;

        menu_updated = 1;
    }

}

void menu_update() {

    // czas pobrany z RTC
    time_t time;
    unsigned char n;

    // czyœæ ekran przed aktualizacj¹ menu
    if ( menu_updated ) {
        lcd_clear();
    }

    // rysuj g³ówne menu
    lcd_xy(0,0);  lcd_char(lcd_larrow);
    lcd_xy(15,0); lcd_char(lcd_rarrow);

    lcd_xy(2,0);

    // ogranicz
    if (menu_pos == 0xff) {
        menu_pos = MENU_POS_COUNT - 1; 
    }
    else if(menu_pos >= MENU_POS_COUNT) {
        menu_pos = 0;
    }

    // wybierz g³ówne menu
    switch(menu_pos) {
        
        // czas i trzy temperatury
        //
        case 0:
            // pobierz czas ...
            ds1306_time_get(&time);

            // ... i poka¿
            lcd_xy(1,0);
            lcd_int2(time.tm_hour); lcd_char(uptime%2 ? ':' : ' '); lcd_int2(time.tm_min);
            lcd_char(' ');//lcd_text_P(PSTR(" 20"));
            lcd_int2(time.tm_year);lcd_char('/');lcd_int2(time.tm_mon);lcd_char('/');lcd_int2(time.tm_mday);

            
            // ogranicz podmenu do liczby czujników
            if (menu_sub_pos == 0xff) {
                menu_sub_pos = 0;
            }
            else if ( (menu_sub_pos > ds_devices_count-3) && (ds_devices_count > 3) ) {
                menu_sub_pos = ds_devices_count-3;
            }

            // poka¿ temperatury
            lcd_xy(1,1);

            for (n=0; n<ds_devices_count && n<3; n++) {
                lcd_int2(abs(ds_temp[menu_sub_pos + n])/10); lcd_char('.'); lcd_char('0' + (abs(ds_temp[menu_sub_pos + n])%10)); lcd_char(' ');
            }

            // strza³ki
            lcd_xy(0,1);  lcd_char( (menu_sub_pos > 0)                  ? lcd_larrow : ' ');
            lcd_xy(15,1); lcd_char( (menu_sub_pos < ds_devices_count-3) ? lcd_rarrow : ' ');

            break;

        // temperatury
        //
        case 1:

            // poka¿ temperatury
            lcd_xy(1,0);

            for (n=0; n<ds_devices_count && n<3; n++) {
                lcd_int2(abs(ds_temp[n])/10); lcd_char('.'); lcd_char('0' + (abs(ds_temp[n])%10)); lcd_char(n<2 ? ' ' : lcd_rarrow);
            }

            lcd_xy(1,1);

            for (n=3; n<ds_devices_count && n<6; n++) {
                lcd_int2(abs(ds_temp[n])/10); lcd_char('.'); lcd_char('0' + (abs(ds_temp[n])%10)); lcd_char(' ');
            }

            break;

        // sieæ (IP, brama, DHCP, pakiety)
        //
        case 2:
            lcd_text_P(PSTR("Sie\x09/")); // sieæ

            // ogranicz podmenu
            if (menu_sub_pos == 0xff) {
                menu_sub_pos = 6;
            }
            else if (menu_sub_pos > 6) {
                menu_sub_pos = 0;
            }

            lcd_xy(7,0);

            switch (menu_sub_pos) {

                case 0:
                    lcd_text_P(PSTR("IP"));

                    lcd_xy(2,1);

                    for (n=0; n<4; n++) {
                        lcd_int(my_net_config.my_ip[n]);
                        lcd_char(n<3 ? '.' : ' ');
                    }
                    break;

                case 1:
                    lcd_text_P(PSTR("brama"));

                    lcd_xy(2,1);
                    for (n=0; n<4; n++) {
                        lcd_int(my_net_config.gate_ip[n]);
                        lcd_char(n<3 ? '.' : ' ');
                    }
                    break;

                case 2:
                    lcd_text_P(PSTR("DHCP"));

                    lcd_xy(2,1);

                    // DHCP
                    if (my_net_config.using_dhcp) {
                        lcd_text_P(PSTR("u\x0fywane"));
                    }
                    else {
                        lcd_text_P(PSTR("nie"));
                    }
                    break;

                case 3:
                    lcd_text_P(PSTR("\x0b\x08")); lcd_text_P(PSTR("cze"));

                    lcd_xy(2,1);

                    // stan ³¹cza
                    if ( enc28_is_link_up() ) {
                        lcd_text_P(PSTR("tak"));
                    }
                    else {
                        lcd_text_P(PSTR("brak"));
                    }
                    break;

                case 4:
                    lcd_text_P(PSTR("pakiety"));

                    lcd_xy(2,1);
                    lcd_text_P(PSTR("Tx: ")); lcd_int(net_ip_packet_id);
                    break;

                case 5:
                    lcd_text_P(PSTR("pakiety"));

                    lcd_xy(2,1);
                    lcd_text_P(PSTR("Rx: ")); lcd_int(my_net_config.pktcnt);
                    break;

                case 6:
                    lcd_text_P(PSTR("MAC"));
                    
                    lcd_xy(2,1);
                    for (n=0; n<6; n++) {
                        lcd_hex(my_net_config.my_mac[n]);
                    }
                    break;
            }

            // strza³ki
            lcd_xy(0,1);  lcd_char(lcd_larrow);
            lcd_xy(15,1); lcd_char(lcd_rarrow);

            break;

        // PID (wysterowania, nastawy, PV, SP)
        //
        /*
        case 3:
            lcd_text_P(PSTR("PID")); // PID

            break;
        */

        // PWM (wype³nienia)
        //
        case 3:
            lcd_text_P(PSTR("PWM"));

            // ogranicz podmenu do liczby kana³ów PWM
            if (menu_sub_pos == 0xff) {
                menu_sub_pos = 0;
            }
            else if (menu_sub_pos > PWM_CHANNELS-3) {
                menu_sub_pos = PWM_CHANNELS-3;
            }

            // nr pokazywanych kana³ów
            lcd_xy(11,0);
            lcd_char('1' + menu_sub_pos);
            lcd_char('-');
            lcd_char('3' + menu_sub_pos);

            // poka¿ wype³nienia
            lcd_xy(2,1);

            for (n=0; n<PWM_CHANNELS && n<3; n++) {
                lcd_char('0' + pwm_get_fill(menu_sub_pos + n)/100);lcd_int2(pwm_get_fill(menu_sub_pos + n)); lcd_char(' ');
            }

            // strza³ki
            lcd_xy(0,1);  lcd_char( (menu_sub_pos > 0)              ? lcd_larrow : ' ');
            lcd_xy(15,1); lcd_char( (menu_sub_pos < PWM_CHANNELS-3) ? lcd_rarrow : ' ');

            break;

        // informacje (wersja, ENC28J60, karta pamiêci, EEPROM)
        //
        case 4:
            lcd_text_P(PSTR("Informacje")); // sieæ

            // ogranicz podmenu
            if (menu_sub_pos == 0xff) {
                menu_sub_pos = 6;
            }
            else if (menu_sub_pos > 6) {
                menu_sub_pos = 0;
            }

            lcd_xy(2,1);

            switch (menu_sub_pos) {

                case 0:
                    lcd_text_P(PSTR(__DATE__));
                    break;

                case 1:
                    lcd_text_P(PSTR("GCC " __AVR_LIBC_VERSION_STRING__));
                    break;

                case 2:
                    //lcd_text_P(PSTR("Up "));

                    // dni
                    lcd_int((uptime/3600/24)); lcd_char('d');
                    // godziny
                    lcd_int2((uptime/3600)%24); lcd_char(':');
                    // minuty
                    lcd_int2((uptime/60)%60); lcd_char(':');
                    // sekundy
                    lcd_int2((uptime%60));

                    break;

                case 3:
                    lcd_text_P(PSTR("ENC28 rev.B"));
                    lcd_char('0' + enc28_read_rev_id());
                    break;

                case 4:
                    lcd_text_P(PSTR("Czujnik\x0dw: "));
                    lcd_int(ds_devices_count);
                    break;

                case 5:
                    lcd_text_P(PSTR("EEPROM "));
                    lcd_int(eeprom_get_size() >> 10);
                    lcd_char('k');lcd_char('B');
                    break;

                case 6:
                    if (sd_get_state() != SD_FAILED) {
                        lcd_text_P(sd_get_state() == SD_IS_SD ? PSTR("SD") : PSTR("MMC"));
                        lcd_char(' ');
                        lcd_int(sd_size >> 10); // kB
                        lcd_char('k');lcd_char('b');
                    }
                    else {
                        lcd_text_P(PSTR("SD/MMC: b\x0b\x08d"));
                    }
                    break;
            }

            // strza³ki
            lcd_xy(0,1);  lcd_char(lcd_larrow);
            lcd_xy(15,1); lcd_char(lcd_rarrow);

            break;
    }

    menu_updated = 0;
}
