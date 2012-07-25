#include "webpage.h"

// naglowki HTTP identyfikujace nasz serwer
const char WEBPAGE_SERVER[] PROGMEM =  "\nServer: Telemetry (" __DATE__ ") ATmega32\n\n";

// cache'owanie
const char WEBPAGE_CACHE[] PROGMEM      = "\nCache-Control: max-age=315360000";
//const char WEBPAGE_DONT_CACHE[] PROGMEM = "\nCache-Control: private, must-revalidate";
const char WEBPAGE_DONT_CACHE[] PROGMEM = "\nCache-Control: max-age=0";

// obsluga zadan HTTP
unsigned int webpage_handle_http(void* tcp, unsigned char* tcp_data)
{
    char query[40];
    unsigned int len = 0;

    rs_text_P(PSTR("HTTP "));

    for (len=0; (tcp_data[len] != 0x0d) && (len < 40); len++)
        rs_send(query[len]=tcp_data[len]);

    query[len] = 0; // zakoncz lancuch NULLem

    rs_newline();


    // rozdziel zapytania HTTP
    // strncasecmp_P (const char * s1, PGM_P s2, size_t n)
    
    // GET
    if (strncasecmp_P(query, PSTR("GET "), 4) == 0) {
        
        //rs_text(query+4); rs_newline();
        
        // strona stertowa
        if (strncasecmp_P(query+4, PSTR("/ "), 2) == 0) {
            return webpage_get_welcome_page(tcp);
        }

        // ustawienia systemu
        else if (strncasecmp_P(query+4, PSTR("/setup "), 7) == 0) {
            return webpage_get_setup_page(tcp);
        }

        // informacje o systemie
        else if (strncasecmp_P(query+4, PSTR("/info "), 6) == 0) {
            return webpage_get_info_page(tcp);
        }

        // podgl¹d stanu / zmiana wype³nieñ PWM
        else if (strncasecmp_P(query+4, PSTR("/pwm "), 5) == 0) {
            return webpage_get_pwm_page(tcp);
        }

        // identyfikacja parametrów systemu (p³ytki grzewczej)
        else if (strncasecmp_P(query+4, PSTR("/ident "), 7) == 0) {
            return webpage_get_ident_page(tcp);
        }

        // akwizycja danych na kartê SD/MMC
        else if (strncasecmp_P(query+4, PSTR("/daq "), 5) == 0) {
            return webpage_get_daq_page(tcp);
        }
        else if (strncasecmp_P(query+4, PSTR("/daq/list "), 10) == 0) {
            return webpage_get_daq_list(tcp);
        }
        else if (strncasecmp_P(query+4, PSTR("/daq/get/"), 9) == 0) {
            len = webpage_get_daq_data(query+13, tcp);

            if (len>0)
                return len;
        }

        // dane statyczne - CSS/JS/PNG/GIF
        else if (strncasecmp_P(query+4, PSTR("/static/"), 8) == 0) {
            len = webpage_get_static_content(query+12, tcp);
            
            if (len>0)
                return len;
        }
    }
    // POST
    else if (strncasecmp_P(query, PSTR("POST "), 5) == 0) {
        
        //rs_text(query+5); rs_newline();

        // JSON
        // {"foo":3,"bar": 3.456}
        if (strncasecmp_P(query+5, PSTR("/json/"), 6) == 0) {
            len = webpage_get_json_content(query+11, tcp);
            
            if (len>0)
                return len;
        }
    }
    
    // naglowek HTTP 404
    len = net_tcp_write_data_P(tcp, 0,   PSTR("HTTP/1.0 404 Not Found\nContent-Type: text/html"));
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_SERVER);
    len = net_tcp_write_data_P(tcp, len, PSTR("<html><head></head><body><strong>404</strong> | boo!</body>\n"));
    
    return len;
}





unsigned int webpage_get_welcome_page(void* tcp) {
    
    unsigned int len = webpage_print_header(tcp, 0);

    // wpisz tresc HTML i otwórz tabelkê na pomiary
    len = net_tcp_write_data_P(tcp, len, PSTR("<h1>Rozk³ad temperatur</h1>\n\n<table id=\"temperatures\" class=\"progress\"><tr>"));

    for (unsigned int dev=0; dev<ds_devices_count; dev++) {
        len = net_tcp_write_data_P(tcp, len, PSTR("\n\t<td>&nbsp;</td>"));
    }

    len = net_tcp_write_data_P(tcp, len, PSTR("\n</tr></table>\n<script>updateTemp()</script>"));

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}



unsigned int webpage_get_setup_page(void* tcp) {
    
   unsigned int len = webpage_print_header(tcp, 0);

    // tabelka z informacjami + JS do jej wype³nienia
    len += firmware_read_sector(3, ((tcp_packet*)tcp)->data + len);

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}

unsigned int webpage_get_pwm_page(void* tcp) {
    
    unsigned int len = webpage_print_header(tcp, 0);

    // tabelka z informacjami o wype³nieniach kana³ów PWM + opcja ich zmiany
    len += firmware_read_sector(4, ((tcp_packet*)tcp)->data + len);

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}

unsigned int webpage_get_info_page(void* tcp) {
    
    unsigned int len = webpage_print_header(tcp, 0);

    // tabelka z informacjami + JS do jej wype³nienia
    len += firmware_read_sector(2, ((tcp_packet*)tcp)->data + len);

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}

unsigned int webpage_get_ident_page(void* tcp) {
    
    unsigned int len = webpage_print_header(tcp, 0);

    // informacja o procedurze identyfikacji + JS do jej przeprowadzenia
    len += firmware_read_sector(5, ((tcp_packet*)tcp)->data + len);

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}


unsigned int webpage_get_daq_page(void* tcp) {
    
    unsigned int len = webpage_print_header(tcp, 0);

    // sprawdŸ stan karty przed wys³aniem odpowiedzi do klienta
    if ( sd_get_state() != SD_FAILED ) {
        // strona z informacj¹ o akwizycji (bie¿¹cy stan, ustawienia nowego zadania...)
        len += firmware_read_sector(6, ((tcp_packet*)tcp)->data + len);
    }
    else {
        // brak / b³¹d karty -> stosowny komunikat
        len += firmware_read_sector(7, ((tcp_packet*)tcp)->data + len);
    }

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}

unsigned int webpage_get_daq_list(void* tcp) {
    
    unsigned int len = webpage_print_header(tcp, 0);

    // sprawdŸ stan karty przed wys³aniem odpowiedzi do klienta
    if ( sd_get_state() != SD_FAILED ) {
        // strona z list¹ plików z pomiarami
        len += firmware_read_sector(8, ((tcp_packet*)tcp)->data + len);
    }
    else {
        // brak / b³¹d karty -> stosowny komunikat
        len += firmware_read_sector(7, ((tcp_packet*)tcp)->data + len);
    }

    // stopka (info o systemie)
    len = webpage_print_footer(tcp, len);

    return len;
}


unsigned int webpage_get_daq_data(char* query, void* tcp) {

    unsigned int len = 0;
    
    char* pos = (char*) strchr(query, ' ');
    *pos = 0;

    rs_send('d'); rs_text(query); rs_newline();

    // nag³ówki
    len = net_tcp_write_data_P(tcp, len, PSTR("HTTP/1.0 200 OK\nContent-Type: text/plain"));
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_SERVER);

    // plik
    fs_file fp;

    // temperatura
    signed int temp;

    // bufor do konwersji liczb na ³añcuch znaków
    char buf[4];
    
    // sprobuj otworzyæ podany plik
    if ( fs_open(&fp, (unsigned char*)query, FS_DONT_CREATE) ) {

        // nag³ówek
        /*
        len = net_tcp_write_data_P(tcp, len, PSTR("#DAQ: "));
        len = net_tcp_write_data(tcp, len, (unsigned char*)query);
        ((tcp_packet*)tcp)->data[len++] = '\n';
        ((tcp_packet*)tcp)->data[len++] = '\n';
        */

        // czytaj ca³y plik
        while( fs_read(&fp, (unsigned char*)(&temp), 2) && len < 1200 ) {
            ltoa(abs(temp)/10, buf, 10);

            // znak - ?
            if (temp < 0) {
                ((tcp_packet*)tcp)->data[len++] = '-';
            }

            // czêœc ca³kowita
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

            // czêœæ u³amkowa
            ((tcp_packet*)tcp)->data[len++] = '.';
            ((tcp_packet*)tcp)->data[len++] = '0' + temp%10;

            // nowa linia
            ((tcp_packet*)tcp)->data[len++] = '\n';
        }

        fs_close(&fp);

        return len;
    }
    else {
        return 0;
    }
}




unsigned int webpage_print_header(void* tcp, unsigned int len)
{
    // nag³ówki HTTP
    len = net_tcp_write_data_P(tcp, len, PSTR("HTTP/1.0 200 OK\nContent-Type: text/html"));
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_DONT_CACHE);   // nie cache'uj!
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_SERVER);       // przedstawmy siê

    // pobierz head.htm z pamieci EEPROM
    len += firmware_read_sector(1, ((tcp_packet*)tcp)->data + len);

    return len;
}


unsigned int webpage_print_footer(void* tcp, unsigned int len)
{
    // stopka (info o systemie)
    len = net_tcp_write_data_P(tcp, len, PSTR("\n\n</div><address>"));
    len = net_tcp_write_data_P(tcp, len, PROGRAM_NAME);
    ((tcp_packet*)tcp)->data[len++] = ' ';
    ((tcp_packet*)tcp)->data[len++] = '(';
    len = net_tcp_write_data_P(tcp, len, PSTR(__DATE__));
    ((tcp_packet*)tcp)->data[len++] = ')';
    /*
    ((tcp_packet*)tcp)->data[len++] = ' ';
    len = net_tcp_write_data_P(tcp, len, PROGRAM_VERSION2);
    */
    len = net_tcp_write_data_P(tcp, len, PSTR("</address></body></html>\n"));

    return len;
}









unsigned int webpage_get_static_content(char* query, void* tcp)
{
    //rs_send('S'); rs_send(' '); rs_text(query); rs_newline();
    unsigned int len = net_tcp_write_data_P(tcp, 0,   PSTR("HTTP/1.0 200 OK"));
    
    // cache dla elementów statycznych
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_CACHE); // cache'uj 10 lat

    // nag³ówki informacyjne serwera
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_SERVER);

    //
    // definicje nr sektorów w skrypcie firmware/loader.php
    //

    // favikonka
    if (strncasecmp_P(query, PSTR("favicon.png"), 11) == 0) {
        len += firmware_read_sector(10, ((tcp_packet*)tcp)->data + len);
    }
    // loading
    else if (strncasecmp_P(query, PSTR("loading.gif"), 11) == 0) {
        len += firmware_read_sector(11, ((tcp_packet*)tcp)->data + len);
    }
    //
    // CSS
    else if (strncasecmp_P(query, PSTR("telemetry.css"), 13) == 0) {
        len += firmware_read_sector(20, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("css.css"), 7) == 0) {
        len += firmware_read_sector(21, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("daq.css"), 7) == 0) {
        len += firmware_read_sector(22, ((tcp_packet*)tcp)->data + len);
    }
    //
    // JS
    else if (strncasecmp_P(query, PSTR("util.js"), 7) == 0) {
        len += firmware_read_sector(30, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("js.js"), 5) == 0) {
        len += firmware_read_sector(31, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("pwm.js"), 6) == 0) {
        len += firmware_read_sector(32, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("ident.js"), 6) == 0) {
        len += firmware_read_sector(33, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("info.js"), 6) == 0) {
        len += firmware_read_sector(34, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("daq.js"), 6) == 0) {
        len += firmware_read_sector(35, ((tcp_packet*)tcp)->data + len);
    }
    else if (strncasecmp_P(query, PSTR("daq_list.js"), 11) == 0) {
        len += firmware_read_sector(36, ((tcp_packet*)tcp)->data + len);
    }
    else {
        return 0;
    }

    return len;
}

unsigned int webpage_get_json_content(char* query, void* tcp)
{
    //rs_send('J'); rs_send(' '); rs_text(query); rs_newline();

    char buf[20];
    unsigned char n;
    
    unsigned int len = net_tcp_write_data_P(tcp, 0,   PSTR("HTTP/1.0 200 OK\nContent-Type: application/json"));
    len = net_tcp_write_data_P(tcp, len, WEBPAGE_SERVER);

    //
    // DS - temperatury z czujników
    //
    if (strncasecmp_P(query, PSTR("ds"), 2) == 0) {
        ((tcp_packet*)tcp)->data[len++] = '[';

        // wpisz temperatury z czujnikow
        for (unsigned int dev=0; dev<ds_devices_count; dev++)
        {
            // indeks
            //((tcp_packet*)tcp)->data[len++] = '0' + dev;
            //((tcp_packet*)tcp)->data[len++] = ':';

            // temperatura (czesc calkowita)
            itoa(ds_temp[dev]/10, buf, 10);
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

            // temperatura (czesc ulamkowa)
            ((tcp_packet*)tcp)->data[len++] = '.';
            ((tcp_packet*)tcp)->data[len++] = '0' + ((ds_temp[dev]%10)%10);
            ((tcp_packet*)tcp)->data[len++] = ',';
            ((tcp_packet*)tcp)->data[len++] = ' ';
        }

        // omiñ ostatni¹ spacjê i przecinek
        len -= 2;

        ((tcp_packet*)tcp)->data[len++] = ']';
    }
    //
    // aktualizacja ustawieñ zegara
    //
    else if (strncasecmp_P(query, PSTR("timer"), 5) == 0) {

        time_t time;
        unsigned long timestamp = 0UL;
        //signed char offset = 0;

        // konwertuj timestamp z ¿¹dania do typu ulong
        unsigned char* pos = (unsigned char*) strchr(query, '_');

        *pos = 0;   // wpisz NULL'a na koñcu wartoœci timestampa
        query += 6; // przesuñ wskaŸnik na wartoœæ timestampa

        //rs_send('=');rs_text(query); rs_newline();

        timestamp = atol(query);

        // konwersja timestamp -> struktura czasu
        gmtime(timestamp, &time);

        /*
        rs_text_P(PSTR("New time from JSON: 20")); rs_int2(time.tm_year); rs_send('-'); rs_int2(time.tm_mon); rs_send('-'); rs_int2(time.tm_mday); rs_send(' ');
            rs_int2(time.tm_hour); rs_send(':'); rs_int2(time.tm_min); rs_send(':'); rs_int2(time.tm_sec); rs_newline();
        */

        // ustaw czas
        ds1306_time_set(&time);

        len = net_tcp_write_data_P(tcp, len, PSTR("{\"result\":1}"));
    }
    //
    // ustawienie wype³nienia / pobranie wype³nieñ kana³ów PWM
    //
    else if (strncasecmp_P(query, PSTR("pwm"), 3) == 0) {

        // ustaw wype³nienie konkretnego kana³u
        // /json/pwm/set/2/123
        if (strncasecmp_P(query+4, PSTR("set"), 3) == 0) {
            unsigned char channel, fill;

            query[9] = 0; // NULL

            // parsuj ¿¹danie
            channel = atoi( (query+8) );
            fill    = atoi( (query+10) );

            // ustaw wype³nienie 
            pwm_set_fill(channel, fill);

            len = net_tcp_write_data_P(tcp, len, PSTR("{\"result\":1}"));

            // debug
            rs_text_P(PSTR("PWM #")); rs_int(channel); rs_send(' '); rs_int(fill);
        }
        // pobierz listê wype³nieñ wszystkich kana³ów
        else {
            ((tcp_packet*)tcp)->data[len++] = '[';

            // wpisz temperatury z czujnikow
            for (unsigned int ch=0; ch<PWM_CHANNELS; ch++)
            {
                // indeks
                //((tcp_packet*)tcp)->data[len++] = '0' + ch;
                //((tcp_packet*)tcp)->data[len++] = ':';

                // wype³nienie (0-255)
                itoa(pwm_get_fill(ch), buf, 10);
                len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

                ((tcp_packet*)tcp)->data[len++] = ',';
                ((tcp_packet*)tcp)->data[len++] = ' ';
            }

            // omiñ ostatni¹ spacjê i przecinek
            len -= 2;

            ((tcp_packet*)tcp)->data[len++] = ']';
        }
    }
    //
    // stan systemu (uptime, odebrane/wys³ane pakiety)
    //
    else if (strncasecmp_P(query, PSTR("status"), 6) == 0) {

        // uptime
        ltoa(uptime, buf, 10);

        len = net_tcp_write_data_P(tcp, len, PSTR("{\"uptime\":"));
        len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

        // pakiety (TX/RX)
        ltoa(net_ip_packet_id, buf, 10);

        len = net_tcp_write_data_P(tcp, len, PSTR(",\"tx\":"));
        len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

        ltoa(my_net_config.pktcnt, buf, 10);

        len = net_tcp_write_data_P(tcp, len, PSTR(",\"rx\":"));
        len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

        // adres MAC
        len = net_tcp_write_data_P(tcp, len, PSTR(",\"mac\":\""));
        for (n=0; n<6; n++) {
            ((tcp_packet*)tcp)->data[len++] = dec2hex(my_net_config.my_mac[n] >> 4);
            ((tcp_packet*)tcp)->data[len++] = dec2hex(my_net_config.my_mac[n] & 0x0f);
            ((tcp_packet*)tcp)->data[len++] = '-';
        }
        len--;

        // adres IP
        len = net_tcp_write_data_P(tcp, len, PSTR("\",\"ip\":\""));
        for (n=0; n<4; n++) {
            itoa(my_net_config.my_ip[n], buf, 10);
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);
            ((tcp_packet*)tcp)->data[len++] = '.';
        }
        len--;

        // DHCP
        len = net_tcp_write_data_P(tcp, len, PSTR("\",\"dhcp\":"));
        ((tcp_packet*)tcp)->data[len++] = my_net_config.using_dhcp ? '1' : '0';

        // czujników DS18B20
        ltoa(ds_devices_count, buf, 10);

        len = net_tcp_write_data_P(tcp, len, PSTR(",\"ds\":"));
        len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

        // karta MMC/SD
        len = net_tcp_write_data_P(tcp, len, PSTR(",\"card\":"));
        
        if (sd_get_state() != SD_FAILED) {
            len = net_tcp_write_data_P(tcp, len, sd_get_state() == SD_IS_SD ? PSTR("\"SD\"") : PSTR("\"MMC\""));

            // rozmiar karty (w kB)
            ltoa(sd_size >> 10, buf, 10);

            len = net_tcp_write_data_P(tcp, len, PSTR(",\"card-size\":"));
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);
        }
        else {
            len = net_tcp_write_data_P(tcp, len, PSTR("false"));
        }

        // rozmiar pamiêci EEPROM
        ltoa((eeprom_get_size() >> 10), buf, 10);

        len = net_tcp_write_data_P(tcp, len, PSTR(",\"eeprom\":"));
        len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

        // zadanie DAQ -> pozosta³o próbek do zebrania
        itoa(daq_task.samples, buf, 10);

        len = net_tcp_write_data_P(tcp, len, PSTR(",\"daq-samples\":"));
        len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

        // zamknij tablicê JS
        ((tcp_packet*)tcp)->data[len++] = '}';
    }
    //
    // ¿¹danie JSON rozpoczêcia akwizycji danych
    //
    // /json/daq/start/pomiar/0/5/200
    //                /<nazwa_pliku>/<nr_kana³u>/<interwa³_pomiarów>/<liczba_próbek>
    //
    else if (strncasecmp_P(query, PSTR("daq/start"), 9) == 0) {

        // sprawdŸ obecnoœæ karty w systemi
        unsigned char result = (sd_get_state() != SD_FAILED) ? 1 : 0;

        if (result) {
            // parsuj zapytanie
            char name[9]; // 8 znaków + NULL
            unsigned char ch;
            unsigned int  interval, samples;

            char* pos;

            // przesuñ wskaŸnik na pocz¹tek ¿¹danej nazwy pliku
            query += 10;

            pos = (char*) strchr(query, '/');
            *pos = 0x00; // wpisz NULL

            memcpy((void*)name, (void*)query, pos-query+1);

            // przesuñ wskaŸnik na nr kana³u
            query = pos+1; 

            pos = (char*) strchr(query, '/');
            *pos = 0x00; // wpisz NULL
            ch = atoi(query);

            // przesuñ wskaŸnik na interwa³
            query = pos+1; 

            pos = (char*) strchr(query, '/');
            *pos = 0x00; // wpisz NULL
            interval = atoi(query);

            // przesuñ wskaŸnik na liczbê sampli
            query = pos+1; 
            samples = atoi(query);

            rs_text(name);rs_send('/');rs_int(ch);rs_send('/');rs_int(interval);rs_send('/');rs_int(samples);rs_newline();

            // spróbuj rozpocz¹æ zadanie akwizycji
            result = daq_start(name, ch, interval, samples);
        }

        len = net_tcp_write_data_P(tcp, len, PSTR("{\"result\":"));
        ((tcp_packet*)tcp)->data[len++] = '0' + result;
        ((tcp_packet*)tcp)->data[len++] = '}';
    }
    //
    // lista plików z pomiarami zapisanych w systemie plików
    //
    else if (strncasecmp_P(query, PSTR("daq/list"), 8) == 0) {
        unsigned long sector = 1;
        fs_file fp;

        ((tcp_packet*)tcp)->data[len++] = '[';
        ((tcp_packet*)tcp)->data[len++] = ' ';

        // szukaj kolejnych plików
        while ( (sector = fs_list_files(&fp, sector)) > 0 ) {
            ((tcp_packet*)tcp)->data[len++] = '[';
            ((tcp_packet*)tcp)->data[len++] = '"';

            // nazwa pliku
            len = net_tcp_write_data(tcp, len, (unsigned char*)fp.name);

            ((tcp_packet*)tcp)->data[len++] = '"';
            ((tcp_packet*)tcp)->data[len++] = ',';

            // sektor
            ltoa(fp.sector, buf, 10);
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);
            ((tcp_packet*)tcp)->data[len++] = ',';

            // rozmiar
            ltoa(fp.size, buf, 10);
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);
            ((tcp_packet*)tcp)->data[len++] = ',';

            // timestamp
            ltoa(fp.timestamp, buf, 10);
            len = net_tcp_write_data(tcp, len, (unsigned char*)buf);

            ((tcp_packet*)tcp)->data[len++] = ']';
            ((tcp_packet*)tcp)->data[len++] = ',';
        }

        len--;

        ((tcp_packet*)tcp)->data[len++] = ']';
    }
    //
    // skasuj wskazany plik
    //
    else if (strncasecmp_P(query, PSTR("daq/delete/"), 11) == 0) {

        len = net_tcp_write_data_P(tcp, len, PSTR("{\"result\":"));

        // nazwa pliku do skasowania
        query += 11;

        char* pos = (char*) strchr(query, ' ');
        *pos = 0;

        fs_file fp;
        
        // spróbuj skasowaæ plik
        if ( fs_open(&fp, (unsigned char*)query, FS_DONT_CREATE) ) {
            fs_delete(&fp);
            ((tcp_packet*)tcp)->data[len++] = '1';
        }
        else {
            ((tcp_packet*)tcp)->data[len++] = '0';
        }

        ((tcp_packet*)tcp)->data[len++] = '}';

    }
    else {
        return 0;
    }

    //rs_dump(((tcp_packet*)tcp)->data, len);

    return len;
}
