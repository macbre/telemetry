#include "config.h"

// struktura w pamiêci EEPROM
config config_eeprom __attribute__((section(".eeprom")));

unsigned char config_read(config* cfg) {

    // inicjalizacja bufora na strukturê ustawieñ
    memset((void*) cfg, 0, sizeof(config));

    eeprom_read_block((void*) cfg, &config_eeprom, sizeof(config));
    return (cfg->header == CONFIG_HEADER) ? 1 : 0;
}

unsigned char config_save(config* cfg) {

    cfg->header = CONFIG_HEADER;

    eeprom_write_block((void*) cfg, &config_eeprom, sizeof(config));
    return 1;
}


void config_menu() {

    unsigned char tmp;

    lcd_clear();
    //               0123456789012345
    lcd_text_P(PSTR("*     Tryb     *"));
    lcd_xy(0,1);
    lcd_text_P(PSTR("* konfiguracji *"));

    // wy³¹cz watchdog'a - bêdziemy czekaæ na dane od u¿ytkownika
    wdt_disable();

    // wy³¹cz przerwania
    cli();

    rs_text_P(PSTR("Konfiguracja systemu:")); rs_newline();
    rs_text_P(PSTR("1) IP systemu")); rs_newline();
    rs_text_P(PSTR("2) IP bramy")); rs_newline();
    rs_text_P(PSTR("3) maska")); rs_newline();
    rs_text_P(PSTR("4) DHCP")); rs_newline();
    rs_text_P(PSTR("5) przypisania czujnikow DS do kanalow")); rs_newline();

    rs_text_P(PSTR("z)apisz ustawienia")); rs_newline();
    rs_text_P(PSTR("r)eset systemu")); rs_newline();

    while(1) {
        rs_newline(); rs_send('>'); rs_send(' ');
        
        switch (rs_recv()) {

            case '1':
                rs_text_P(PSTR("Podaj nowy adres IP systemu: "));
                config_ip(my_config.my_ip);
                break;
        
            case '2':
                rs_text_P(PSTR("Podaj nowy adres IP bramy: "));
                config_ip(my_config.gate_ip);
                break;

            case '3':
                rs_text_P(PSTR("Podaj now¹ maskê IP: "));
                config_ip(my_config.mask);
                break;

            case '4':
                rs_text_P(PSTR("W³¹czyæ obs³ugê DHCP? (t/n) "));
                
                if ( config_ask() ) {
                    my_config.config |= CONFIG_USE_DHCP;
                }
                else {
                    my_config.config &= ~CONFIG_USE_DHCP;
                }

                break;

            case '5':
                // patrz komentarz w ds18b20.c
                for (tmp=0; tmp < ds_devices_count; tmp++) {
                    rs_newline();
                    rs_text_P(PSTR("Podaj przypisanie kanalu dla czujnika #"));
                    rs_int(tmp);

                    rs_send(' ');
                    rs_send('[');
                    rs_int(my_config.ds_assignment[tmp]);
                    rs_send(']');

                    my_config.ds_assignment[tmp] = config_get_num();
                }
                
                break;

            case 'z':
                rs_text_P(PSTR("Zapisaæ ustawienia? (t/n) "));

                //rs_dump((void*) &my_config, sizeof(config));

                if ( config_ask() ) {
                    config_save(&my_config);

                    rs_newline();
                    rs_text_P(PSTR("Ustawienia zosta³y zapisane. Zresetuj system!"));
                }

                break;

            case 'r':
                soft_reset();
                break;

            default:
                break;
        }
    }

}

unsigned char config_ask() {

    unsigned char ch;

    do {
        ch = rs_recv();
    } while (ch != 't' && ch != 'n');

    rs_send(ch);

    return (ch == 't') ? 1 : 0;
}

void config_ip(uint8_t* ip) {

    unsigned char ch;
    unsigned char buf[4];
    unsigned char pos;

    // wprowadzaj kolejne czêœci adresu IP (xxx.xxx.xxx.xxx)
    for (unsigned char p=0; p<4; p++) {

        memset((void*)buf, 0, 4);

        pos = 0;

        // czekaj na wprowadzenie fragmentu adresu IP (czekaj na trzy cyfry lub kropkê)
        while (pos < 3) {
            ch = rs_recv();
            if ( ch >= '0' && ch <= '9' ) {
                rs_send(ch);
                buf[pos++] = ch;
            }
            if ( ch == '.' && pos > 0 ) {
                pos = 3;
            }
        }

        if (p<3)
            rs_send('.'); 

        // parsuj wprowadzone dane
        *(ip+p) = atoi((char*)buf);
    }

    rs_newline(); net_dump_ip(ip);
}

unsigned char config_get_num() {

    unsigned char ch;

    do {
        ch = rs_recv();
    } while (ch < '0' || ch > '9');

    rs_send(ch);

    return (ch - '0');
}
