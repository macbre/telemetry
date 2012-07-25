/*
 * System telemetryczny z zastosowaniem technologii internetowych
 *
 * 2007-2008 (c) Maciej Brencz
 *
 */

#include "telemetry.h"

const char PROGRAM_NAME[] PROGMEM =      "Telemetria v1.0";
const char PROGRAM_COPYRIGHT[] PROGMEM = "Maciej Brencz 2007/08";
const char PROGRAM_VERSION1[] PROGMEM =  "Wersja "  __DATE__ " " __TIME__;
const char PROGRAM_VERSION2[] PROGMEM =  "AVR-GCC v" __AVR_LIBC_VERSION_STRING__ " (" __AVR_LIBC_DATE_STRING__ ")";

unsigned char tmp = 0;

// fuse bity - konfiguracja uC
/*
FUSES =
{
    .low = (BODLEVEL & SUT1 & SUT0 & CKSEL3 & CKSEL2 & CKSEL1 & CKSEL0),
    .high = (OCDEN & JTAGEN & CKOPT & EESAVE & BOOTRST),
};
*/
// inicjalizacja watchdog'a
void wdt_init(void)
{
    MCUCSR = 0;
    wdt_disable();
}

// ----------------------------------------------------------------------------------------------------------------
// inicjalizacja przerwan
void init_interrupts()
{
    //
    // PWM DRIVER: przerwanie od timera0
    //
    OCR0 = 0x80;                            // ~ 2 ms
    TIMSK |= (1 << OCIE0);                  // przerwanie od Output Compare (TCNT0 == OCR0)
    TCCR0 |= (1 << WGM01);                  // Clear Timer on Compare (CTC)
    TCCR0 |= (1 << CS02) | (1 << CS12);     // preskaler CK/1024
    TCNT0 = 0x00;                           // zeruj zegar

    //
    // POOLING: przerwanie od timera1
    //
    TIMSK |= (1 << TOIE1);	// Timer Overflow Interrupt Enabled

	TCCR1B |= (1 << CS10) | (1 << CS12);	// preskaler CK/1024

	// Preskaler: 1MHz / 1024 ~ 976,56 Hz
	//
	// x taktow => 125 ms
	// 1 takt   => 0,00102 s ~ 1 ms

	// odlicz czas (25 ms)
    TCNT1   = 0xffff - 25*16; // 16 MHz

    //
    // DS1306: przerwanie zboczem 1->0 na INT0
    //
    MCUCR |= (1 << ISC01); // zbocze opadaj¹ce na INT0
    GICR  |= (1 << INT0);  // zgoda na INT0

    DDRD &= ~(1<<2); // INT0 jako wejscie
    PORTD |= (1<<2); //      z podci¹ganiem

    //
    // ENC28: przerwania zboczem 1->0 na INT1
    //
    MCUCR |= (1 << ISC11); // zbocze opadajace na INT1
    GICR  |= (1 << INT1);  // zgoda na INT1

    DDRD &= ~(1<<3); // INT1 jako wejscie
}


// ----------------------------------------------------------------------------------------------------------------
// przerwanie od CTC Timera0
//
// aktualizacja wyjœæ sterownika PWM
ISR(SIG_OUTPUT_COMPARE0)
{
    pwm_loop();
}



// ----------------------------------------------------------------------------------------------------------------
// przerwanie od przepelnienia Timera1 (co 25 ms)
//
// w trybie poolingu:
//  * aktualizuj wartosci temperatur z czujnikow
//  * aktualizuj czas na wysw. LCD
ISR(SIG_OVERFLOW1)
{
    // odliczaj kolejny okres
    TCNT1   = 0xffff - 25*16;   // 16 MHz

    // pytaj okresowo o czas
    static unsigned int last_ntp_update;

    // odbior danych z RS'a
    if (rs_has_recv()) {
        on_rs_cmd( rs_recv() );
    }

    // ENC28: sprawdŸ czy nie ma w buforze kolejnych pakietów do obs³u¿enia
    if (enc28_count_packets() > 0) {
        on_int1();
    }

    // skanuj przyciski klawiatury
    keys_scan();

    if ( keys_pressed() ) {
        // aktualizuj menu wg wybranego przycisku
        menu_handle_keys();
    }


    //
    // pooling
    //

    // zwiêksz co 25 ms
    if (pooling_step++ > 40) {
        pooling_step = 0;
    }
    if (pooling_step % 5)
        return;

    // wybierz krok poolingu do obsluzenia
    switch((pooling_step/5)%8) {

        case 0:
        //case 4:
            // odczyt temperatury z wszystkich czujnikow
            ds18b20_get_temperature_from_all();

            // wyœlij zadanie kolejnego pomiaru temperatury do wszystkich czujnikow ds18b20
            ds18b20_request_measure();

            break;

        case 2:
            // akwizycja danych na kartê pamiêci
            daq_pooling();
            break;

        case 3:
            // brak MAC'a bramy? -> nie wyœlemy pakietu w œwiat
            if (my_net_config.gate_mac[0] == 0xff)
                break;

            // proba aktualizacji timera NTP ?
            if (last_ntp_update == 0) {
                net_ntp_get_time(net_packet);

                last_ntp_update = 0xffff - (15*60); // odswiezaj czas co 15 minut

                rs_text_P(PSTR("NTP: wyslane zapytanie")); rs_newline();
            }

            last_ntp_update++;

            break;
    }

    // aktualizuj wskazania menu
    //
    // po wciœniêciu przycisku i co 250 ms
    if ( keys_pressed() || ((pooling_step%10) == 0) ) {
        menu_update();
    }
    
}


// ----------------------------------------------------------------------------------------------------------------
// przerwania zboczem opadajacym na INT0 (inkrementacja uptime'u co 1 sekundê)
ISR(SIG_INTERRUPT0)
{
    uptime++;
}

// ----------------------------------------------------------------------------------------------------------------
// przerwania zboczem opadajacym na INT1 (odbior pakietow z ENC28)
ISR(SIG_INTERRUPT1)
{
    on_int1();
}

void on_int1()
{
    cli();

    // dlugosc odebranego pakietu
    unsigned int len;

    // definicje pakietow
    ethernet_packet* eth_packet;
        
    // odbierz i "parsuj" pakiet
    len = enc28_packet_recv(net_packet, ENC28_MAX_FRAMELEN);

    // niczego nie odebralismy
    if (len == 0) {
        sei();
        return;
    }

    //rs_newline(); rs_send('>'); rs_send(' '); rs_int(enc28_count_packets()); rs_send('w'); rs_send(' ');
    //rs_newline(); rs_send('>'); rs_int(len); rs_send('b'); rs_send(' ');
    
    eth_packet = net_handle_eth_packet(net_packet, len);

    // obsluz pakiet "wewnatrz" biblioteki net.h (DHCP/ARP/ICMP/NTP)
    if ( net_stack(eth_packet) == NET_STACK_HANDLED ) {
        // ...
    }
    // obslugujemy tylko pakietu IPv4
    else if ( HTONS(eth_packet->eth_type) == NET_IP4_FRAME ){

        //net_dump_eth_packet(eth_packet, len);

        // pakiet IP
        ip_packet* ip = (ip_packet*) (eth_packet->data);
        
        // dane IP
        unsigned char* ip_data = (((unsigned char*) ip) + ((ip->ver_ihl & 0x0f) * 4) );

        // port docelowy pakietu UDP/TCP
        unsigned int port;

        //
        // obsluz pakiety UDP
        //

        if (ip->proto == NET_IP_UDP) {
            port = HTONS( ((udp_packet*) ip_data)->dest_port );

            unsigned char *udp_data = ((udp_packet*) ip_data)->data;

            // kieruj na odpowiedni port UDP
            switch(port) {
                // aktualizacja firmware'u
                case FIRMWARE_PORT:
                    len = firmware_handle_packet(udp_data);
                    break;

                // DAQ
                case DAQ_PORT:
                    len = daq_handle_packet(udp_data);
                    break;


                default:
                    len = 0;
            }

            if (len > 0)
                len = net_make_udp_packet(((udp_packet*) ip_data), port, HTONS(((udp_packet*) ip_data)->src_port), udp_data, len);
       
        }


        //
        // obsluz pakiety TCP
        //

        else if (ip->proto == NET_IP_TCP) {
            port = HTONS( ((tcp_packet*) ip_data)->dest_port );

            rs_text_P(PSTR("TCP od ")); net_dump_ip(ip->src_addr);rs_send(':'); rs_int(port); rs_newline();

            unsigned char *tcp_data = ip_data + ( ((tcp_packet*) ip_data)->hlen >> 4) * 4;

            // kieruj na odpowiedni port TCP
            switch(port) {
                // HTTP
                case WEBPAGE_PORT:
                case WEBPAGE_PORT_ALT:
                    len = webpage_handle_http((tcp_packet*) ip_data, tcp_data);
                    break;
                
                default:
                    len = 0;
            }

            if (len > 0)
                len = net_make_tcp_packet(((tcp_packet*) ip_data), NET_TCP_FLAG_FIN | NET_TCP_FLAG_PUSH | NET_TCP_FLAG_ACK, port, HTONS(((tcp_packet*) ip_data)->src_port), NULL, len);
        
        }
        // bledne zadanie
        else {
            len = 0;
        }

        // wyslij pakiet z odpowiedzia (jesli jest cos do wyslania)
        if (len > 0) {
            len = net_make_ip_packet(ip, ip->proto, ip_data, ip->src_addr, len);
            len = net_make_eth_packet(eth_packet, (uint8_t*) ip, eth_packet->src, len);

            enc28_packet_send((unsigned char*)eth_packet, len);
            rs_newline(); //rs_send('<');rs_int(len);rs_newline();
        }
    }

    sei();
}

// obsluga komend z terminala RS
void on_rs_cmd(unsigned char cmd)
{
    rs_send('#');rs_send(' '); rs_send(cmd); rs_newline();

    switch(cmd) {
        // zrzut rejestrow ENC28J60
        /*
        case 'e':
            enc28_dump();
            break;
        */
        // konfiguracja systemu
        case 'k':
            config_menu();
            break;

        // aktualne pomiary temperatur
        case 't':
            for (unsigned char dev=0; dev<ds_devices_count; dev++) {
                rs_int(abs(ds_temp[dev])/10); rs_send('.'); rs_send('0' + (abs(ds_temp[dev])%10)); rs_send(' ');
            }
            rs_newline();
            break;

        // aktualne wype³nienia PWM
        case 'p':
            for (unsigned char ch=0; ch<PWM_CHANNELS; ch++) {
                rs_int(pwm_get_fill(ch)); rs_send(' ');
            }
            rs_newline();
            break;

        // ustawienia stosu TCP/IP
        case 'n':
            net_dump_info();
            break;

        // reset systemu
        case 'r':
            rs_newline(); rs_send('r'); rs_send('!'); rs_newline(); 
            soft_reset();
            break;

        // uptime systemu
        case 'u':
            rs_int((uptime/3600/24)); rs_send('d');
            // godziny
            rs_int2((uptime/3600)%24); rs_send(':');
            // minuty
            rs_int2((uptime/60)%60); rs_send(':');
            // sekundy
            rs_int2((uptime%60));

            rs_newline();
            break;

        // DHCP
        /*
        case 'd':
            net_dhcp_init(net_packet);
            break;
        */

        default:
            rs_newline();
            rs_text_P(PROGRAM_NAME);    rs_newline(); rs_newline();

            rs_text_P(PSTR("k - konfiguracja systemu"));        rs_newline();
            rs_text_P(PSTR("n - ustawienia stosu TCP/IP"));     rs_newline();
            rs_text_P(PSTR("p - wypelnienia kanalow PWM"));     rs_newline();
            rs_text_P(PSTR("r - reset systemu"));               rs_newline();
            rs_text_P(PSTR("t - pomiary temperatur"));          rs_newline();
            rs_text_P(PSTR("u - uptime systemu"));              rs_newline();
    }

    rs_newline();
}

// ----------------------------------------------------------------------------------------------------------------
// inicjalizacja peryferiow
void init_peripherals()
{
    unsigned char i;

    // -----------------------------------------------------------------------------------------
    // RS
    rs_init(115200L);   // 115 kbps

    rs_text_P(PROGRAM_NAME);        rs_newline();
    rs_text_P(PROGRAM_COPYRIGHT);   rs_newline();
    rs_text_P(PROGRAM_VERSION1);    rs_newline();
    rs_text_P(PROGRAM_VERSION2);    rs_newline();
    rs_text_P(PSTR("--------"));    rs_newline();rs_newline();

    // -----------------------------------------------------------------------------------------
    // wyswietlacz LCD
    lcd_init();
    lcd_text_P(PROGRAM_NAME);
    lcd_xy(0,1);
    lcd_char(lcd_block);

    // -----------------------------------------------------------------------------------------
    // klawiatura ze switch'y
    keys_init();
    lcd_char(lcd_block);

    // -----------------------------------------------------------------------------------------
    // menu
    menu_init();
    lcd_char(lcd_block);

    // -----------------------------------------------------------------------------------------
    // PWM
    pwm_init();
    lcd_char(lcd_block);

    // -----------------------------------------------------------------------------------------
    // odczyt ustawieñ z pamiêci EEPROM
    rs_text_P(PSTR("EEPROM: "));

    if ( config_read(&my_config) ) {
        //
        // uda³o siê odczytaæ ustawienia -> wprowadŸ je
        //
        rs_text_P(PSTR("odczytano ustawienia systemu")); rs_newline();
    }
    else {
        //
        // domyœlne wartoœci ustawieñ
        //

        // MAC i domyslne IP systemu
        mac_addr mac = NET_MAC;
        ip_addr  ip  = NET_IP;

        memcpy((void*) (my_config.my_mac), (void*)mac, sizeof(mac_addr));
        memcpy((void*) (my_config.my_ip),  (void*)ip, sizeof(ip_addr));

        // brama -> x.x.x.1        
        memcpy((void*) (my_config.gate_ip),(void*)ip, sizeof(ip_addr));
        my_config.gate_ip[3] = 1;

        // maska -> 255.255.255.00
        my_config.mask[0] = 255;
        my_config.mask[1] = 255;
        my_config.mask[2] = 255;
        my_config.mask[3] = 0;

        // dodatkowe ustawienia (brak DHCP)
        my_config.config = 0;

        // przypisania czujników DS do kana³ów
        memset((void*) (my_config.ds_assignment), 0, DS_DEVICES_MAX);

        config_save(&my_config);

        rs_text_P(PSTR("wprowadzono domyœlne ustawienia systemu")); rs_newline();
    }
    //rs_dump((void*) &my_config, sizeof(config));
    rs_newline();
    lcd_char(lcd_block);

    // -----------------------------------------------------------------------------------------
    // PID
    /*
    for (i=0; i < PID_COUNT; i++)
        pid_init(&pid[i], 5.0*PID_SCALING_FACTOR, 5.8*PID_SCALING_FACTOR, 3.2*PID_SCALING_FACTOR);
    */

    // -----------------------------------------------------------------------------------------
    // SPI
    spi_master_init();
    lcd_char(lcd_block);

    rs_text_P(PSTR("SPI\t\t[ok]"));

    // -----------------------------------------------------------------------------------------
    // -- karta SD/MMC
    unsigned char sd_ok = sd_init();
    unsigned char sd_result = sd_get_state();
    
    rs_newline();
    rs_text_P(PSTR("\t* ")); //SD/MMC"));

    // inicjalizacja - ok
    if (sd_result != SD_FAILED) {
        rs_text_P(sd_result == SD_IS_SD ? PSTR("SD") : PSTR("MMC"));
        rs_text_P( PSTR(" [OK]"));

        // pobierz informacje o karcie
        sd_cardid sd_card;

        if ( sd_info(&sd_card) == 1 ) {

            // pokaz nazwe karty
            rs_send(' ');
            for(i=0; i<5; i++) {
                rs_send(sd_card.name[i]);
            }

            // rozmiar
            rs_send(' '); rs_long(sd_card.capacity >> 10); rs_send('k'); rs_send('B');
        }

        // system plików (FS) -> ustaw jako bufor operacji I/O koniec bufora na ramkê ethernetow¹
        //
        if ( fs_init(net_packet+1000) ) {
        /**
            fs_file fp;
            unsigned char data;
            
            fs_open(&fp, (unsigned char*)"pomiar");
            rs_newline(); rs_int(fp.sector); rs_send('-'); rs_int(fp.size);

            fs_write(&fp, (unsigned char*)"foobar", 6);

            while( fs_read(&fp, &data, 1) ) {
                rs_send(data);
            }

            fs_close(&fp);
        **/
        }

        // inicjalizacja FAT'a
        /*
        if ( fat_init(&fat, net_packet+512) ) {
            rs_send(' ');
            rs_text((char*)fat.name);

            // test otwarcia pliku
            fat_file fp;

            // test.txt
            fat_file_open(&fp, (unsigned char*)"test    ", (unsigned char*)"txt");
            rs_int(fp.size);

            // INDEX~1.HTM
            fat_file_open(&fp, (unsigned char*)"index~1 ", (unsigned char*)"htm");
            rs_int(fp.size);
        

            // __teleme.try
            fat_file_open(&fp, (unsigned char*)"__teleme", (unsigned char*)"try");
            rs_int(fp.size);
        }
        */
    }
    // b³¹d
    else {
        rs_text_P(PSTR("SD/MMC [err! 0x")); rs_hex(sd_ok); rs_send(']');
    }

    // -----------------------------------------------------------------------------------------
    // -- Ethernet (ENC28J60)
    enc28_init();   

    rs_newline();
    rs_text_P(PSTR("\t* ETH: enc28j60"));
    enc28_read_rev_id();

    lcd_char(lcd_block);


    // -----------------------------------------------------------------------------------------
    // -- EEPROM 25LC1024
    eeprom_init();

    rs_newline();
    rs_text_P(PSTR("\t* EEPROM [id 0x")); rs_hex( eeprom_read_signature() ); rs_send(']'); rs_send(' ');
    rs_int( eeprom_get_size() >> 10 );
    rs_text_P(PSTR(" kB"));

    // -----------------------------------------------------------------------------------------
    // -- RTC DS1306
    ds1306_init();  
    lcd_char(lcd_block);

    rs_newline();
    rs_text_P(PSTR("\t* RTC: DS1306"));rs_newline();


    // -----------------------------------------------------------------------------------------
    // 1wire - DS18B20
    unsigned char ow_slave_present = ow_reset();

    rs_newline();
    rs_text_P(PSTR("1WIRE:  reset magistrali (urzadzenia slave "));
    rs_text_P(ow_slave_present ? PSTR("podlaczone)") : PSTR("nie podlaczone)"));
    rs_newline();

    // detekcja slave'ow na 1wire
    //ds18b20_init(DS18B20_RESOLUTION_11_BITS);
    ds18b20_init(DS18B20_RESOLUTION_12_BITS);

    // wylistuj czujniki DS18B20 z ich kodami ROM
    rs_text_P(PSTR("\t* DS18B20 x")); rs_int( ds_devices_count ); rs_newline();

    for (i=0; i < ds_devices_count; i++) {
        rs_text_P(PSTR("\t\t- #")); rs_int(i); rs_send(' ');
        
        rs_hex(ds_devices[i][0]); rs_send(':');

        for (tmp=1; tmp < 7; tmp++)
            rs_hex(ds_devices[i][tmp]);

        rs_send(' '); rs_send('@'); rs_int(my_config.ds_assignment[i]);

        rs_newline();
    }
    lcd_char(lcd_block);
    

    // -----------------------------------------------------------------------------------------
    // warstwa sieciowa
    rs_newline();
    rs_text_P(PSTR("ETHERNET"));

    net_packet[0] = 0;

    enc28_net_init(my_config.my_mac, my_config.my_ip);   // inicjalizacja warstwy sieciowej ENC28

    rs_text_P(PSTR("\t\t[ok]"));
    rs_newline();

    rs_text_P(PSTR("\t* MAC ")); net_dump_mac(my_net_config.my_mac); rs_newline();
    rs_text_P(PSTR("\t* IP "));  net_dump_ip(my_net_config.my_ip);   rs_newline();

    rs_text_P(PSTR("\t* enc28j60 rev. B")); rs_send('0' + enc28_read_rev_id());

    rs_newline();rs_newline();

    lcd_char(lcd_block);

    // -----------------------------------------------------------------------------------------
    // koniec inicjalizacji
    rs_text_P(PSTR("INIT\t\t[ok]")); rs_newline(); rs_newline();

    lcd_clear();

    // w³¹cz menu
    menu_update();
}

// ----------------------------------------------------------------------------------------------------------------
// glowny program
int main()
{
    // watchdog: wy³¹cz!
    wdt_init();

    // inicjalizacja zmiennych globalnych
    uptime = 0;
    pooling_step = 0;

    init_peripherals(); // uk³ady peryferyjne
    init_interrupts();  // przerwania

    // od teraz przyjmuj zgloszenia przerwan...
    sei(); 

    // DHCP?
    if (my_config.config & CONFIG_USE_DHCP) {
        // wyslij zapytanie DHCP
        net_dhcp_init(net_packet);
    }
    else {
        // nie u¿ywamy DHCP: wpisz dane bramy z usa=tawieñ systemu
        memcpy((void*) my_net_config.gate_ip, (void*) my_config.gate_ip, sizeof(ip_addr));
        my_net_config.using_dhcp = 0;

        // zapytaj o adres MAC bramy
        net_arp_ask((ip_addr*) my_net_config.gate_ip, (ethernet_packet*) net_packet);
    }

    // czekaj na zgloszenia przerwan i zajmuj sie ich obsluga
    for(;;);

    return 1;
}
