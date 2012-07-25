#include "net.h"

// -----------------------------------------------------------------------------------------
// funkcje narzedziowe

uint8_t net_is_my_ip(uint8_t* ip)
{
    unsigned char i;

    // sprawdŸ zgodnosc kolejnych bajtow
    for (i=0; i<4; i++)
        if ( (ip[i] != my_net_config.my_ip[i]) && (ip[i] != 0xff) )
            return 0;

    return 1;
}

uint8_t net_is_my_mac(uint8_t* mac)
{
    unsigned char i;

    // sprawdŸ zgodnosc kolejnych bajtow + zwroc true dla broadcast (ff-ff-ff-ff-ff-ff)
    for (i=0; i<6; i++)
        if ( (mac[i] != my_net_config.my_mac[i]) && (mac[i] != 0xff) )
            return 0;

    return 1;
}

uint16_t htons(uint16_t val)
{
	return (val<<8) | (val>>8);
}

uint32_t htonl(uint32_t val)
{
	return (htons(val>>16) | (uint32_t)htons(val&0x0000FFFF)<<16);
}

// -----------------------------------------------------------------------------------------
// debug

void net_dump_eth_packet(ethernet_packet* packet, uint16_t len)
{
    // ogranicz wielkosc pakietu
    len = (ENC28_MAX_FRAMELEN < len) ? ENC28_MAX_FRAMELEN : len;

    rs_newline();
    rs_text_P(PSTR("Packet dump (")); rs_int(len); 
    rs_text_P(PSTR(" bytes, type ")); rs_hex(packet->eth_type); rs_hex(packet->eth_type >> 8); rs_send(' ');

    switch(HTONS(packet->eth_type)) {
        case NET_ARP_FRAME:
            rs_text_P(PSTR("ARP"));
            break;

        case NET_IP4_FRAME:
            rs_text_P(PSTR("IPv4"));
            break;

        default:
            rs_text_P(PSTR("[unknown]"));
     }

    rs_send(')');rs_newline();

    rs_text_P(PSTR("Src: ")); net_dump_mac(packet->src);
    rs_text_P(PSTR(" / dest: "));  net_dump_mac(packet->dest);
    rs_newline();rs_newline();

    // zrzut zawartosci pakietu (data)
    if (len > 15)
        rs_dump(packet->data, len-14);
}

void net_dump_ip(ip_addr ip)
{
    for (unsigned char i=0; i<4; i++) {
        rs_int(ip[i]);
        if (i<3) rs_send('.');
    }
}

void net_dump_mac(mac_addr mac)
{
    for (unsigned char i=0; i<6; i++) {
        rs_hex(mac[i]);
        if (i<5) rs_send('-');
    }
}

void net_dump_info() {
    rs_text_P(PSTR("MAC     "));  net_dump_mac(my_net_config.my_mac);  rs_newline();
    rs_text_P(PSTR("IP      "));  net_dump_ip(my_net_config.my_ip);    rs_newline();
    rs_text_P(PSTR("Maska   "));  net_dump_ip(my_net_config.mask);     rs_newline();
    rs_text_P(PSTR("Brama   "));  net_dump_ip(my_net_config.gate_ip);  rs_send('/'); net_dump_mac(my_net_config.gate_mac); rs_newline();
    rs_text_P(PSTR("DHCP    "));  rs_text_P( my_net_config.using_dhcp ? PSTR("tak") : PSTR("nie") ); rs_newline();
    rs_text_P(PSTR("Pakiety "));  rs_send('T');rs_int(net_ip_packet_id); rs_send('/'); rs_send('R');rs_int(my_net_config.pktcnt);  rs_newline();
    rs_text_P(PSTR("Duplex  "));  rs_text_P( my_net_config.using_fullduplex ? PSTR("full") : PSTR("half") ); rs_newline();
    rs_text_P(PSTR("Link    "));  rs_text_P( enc28_is_link_up() ? PSTR("tak") : PSTR("nie") ); rs_newline();
}


// -----------------------------------------------------------------------------------------
// obliczanie sumy kontrolnej pakietow
// @see rfc 1071
uint16_t net_checksum(void *data, uint16_t len, uint8_t proto)
{
    register uint32_t sum = 0;

    // dodaj pseudo-naglowek 96 bitowy dla TCP/IP
    if (proto != 0) {
        sum += HTONS((len-8));  // dlugosc naglowka TCP/IP (big-endian)
        sum += HTONS(proto);    // protokol IP (big-endian)
    }

    for (;;) {
        if (len < 2)
            break;
		sum += *((uint16_t *)data);
		data+=2;
        len -= 2;
    }

    // pozostaly "nieparzysty" bit
    if (len)
        sum += *(uint8_t *) data;

    // suma 32 bitowa na wartosc 16 bitowa
    while ((len = (uint16_t) (sum >> 16)) != 0)
        sum = (uint16_t) sum + len;

    return (uint16_t) sum ^ 0xFFFF;
}


// -----------------------------------------------------------------------------------------
// zwroc dane z pakietu ethernetowego

ethernet_packet* net_handle_eth_packet(uint8_t* data, uint8_t len)
{
    return (ethernet_packet*) data;
}

// -----------------------------------------------------------------------------------------
// tworz pakiet ethernetowy dla podanych danych

uint16_t net_make_eth_packet(ethernet_packet* eth, uint8_t* data, uint8_t* dest, uint16_t len)
{
    unsigned char i;

    // wpisz adresy MAC nadawcy i odbiorcy
    for (i=0; i<6; i++) {
        eth->dest[i] = dest[i];
        eth->src[i]  = my_net_config.my_mac[i];
    }
   
    return len + 14; // dlugosc pakietu + dlugosc mac * 2 + typ pakietu
}

// -----------------------------------------------------------------------------------------
// tworz pakiet IP dla podanych danych

uint16_t net_make_ip_packet(ip_packet* ip, uint8_t proto, uint8_t* data, uint8_t* dest, uint16_t len)
{
    unsigned char i;

    // wpisz adresy IP nadawcy i odbiorcy
    for (i=0; i<4; i++) {
        ip->dest_addr[i] = dest[i];
        ip->src_addr[i]  = my_net_config.my_ip[i];
    }

    // zwieksz licznik wyslanych pakietow
    net_ip_packet_id++;

    // ustaw inne pola naglowka IP
    ip->ver_ihl      = 0x45;
    ip->tos          = 0;
    ip->length       = HTONS( (20 + len) );
    ip->id           = HTONS(net_ip_packet_id);
    ip->flags_offset = HTONS( (0x4000) ); // nie fragmentuj pakietow (DF)
    ip->ttl          = 64;
    ip->proto        = proto;

    // oblicz sume kontrolna naglowka IP
    ip->checksum = 0;
    ip->checksum = net_checksum((uint8_t*)ip, 20, 0);

    // oblicz sume kontrolna niesionych pakietow UDP/TCP
    if (proto == NET_IP_UDP) {
        // przelicz sume kontrolna pakietu UDP
        udp_packet* udp = (udp_packet*) ip->data;

        // zeruj sume kontrolna przed jej obliczeniem
        udp->checksum = 0;
        udp->checksum = net_checksum( ((uint8_t*)udp) - 8, 8+len, NET_IP_UDP); // 8+len - dlug. TCP + adresy IP nadawcy/odbiorcy + dlugosc IP + proto
    }

    if (proto == NET_IP_TCP) {
        // przelicz sume kontrolna pakietu TCP
        tcp_packet* tcp = (tcp_packet*) ip->data;

        // zeruj sume kontrolna przed jej obliczeniem
        tcp->checksum = 0;
        tcp->checksum = net_checksum( ((uint8_t*)tcp) - 8, 8+len, NET_IP_TCP); // 8+len - dlug. TCP + adresy IP nadawcy/odbiorcy + dlugosc IP + proto
    }

    return 20 + len;
}

// -----------------------------------------------------------------------------------------
// tworz pakiet UDP dla podanych danych

uint16_t net_make_udp_packet(udp_packet* udp, uint16_t src_port, uint16_t dest_port, uint8_t* data, uint16_t len)
{
    // wpisz numery portow
    udp->src_port  = HTONS(src_port);
    udp->dest_port = HTONS(dest_port);
    udp->length    = HTONS( (8 + len) );
    
    // !!! suma kontrolna liczona w funkcji obslugi "wyzszego" pakietu IP
    udp->checksum = 0;

    return 8 + len;
}

// -----------------------------------------------------------------------------------------
// tworz pakiet TCP dla podanych danych

uint16_t net_make_tcp_packet(tcp_packet* tcp, uint8_t flags, uint16_t src_port, uint16_t dest_port, uint8_t* data, uint16_t len)
{
    // wpisz numery portow, ustaw flagi
    tcp->src_port  = HTONS(src_port);
    tcp->dest_port = HTONS(dest_port);
    tcp->flags     = flags;

    // rozmiar okna i naglowka
    tcp->window_size = HTONS(5840);
    tcp->hlen = 0x60; /* (24/4) << 4 */

    // przepisz numer potwierdzenia do numeru sekwencji i wpisz numer potwierdzenia = numer sekwencji + 1
    unsigned long ack_num = tcp->ack_num;

    tcp->ack_num = htonl(tcp->seq_num) + 1UL;
    tcp->ack_num = htonl(tcp->ack_num);

    tcp->seq_num = ack_num;

    tcp->options = 0x80050402; // opcje: max. rozmiar segmentu danych: 1408 bajtow
    
    // !!! suma kontrolna liczona w funkcji obslugi "wyzszego" pakietu IP
    tcp->checksum = 0;

    return 24 + len;
}

// -----------------------------------------------------------------------------------------
// synchronizuj z hostem TCP

void net_tcp_synchronize(tcp_packet* tcp, ethernet_packet* eth)
{
    unsigned int len;

    ip_packet  *ip   = (ip_packet*) (eth->data);

    // przepisz numery portow
    len = tcp->src_port;
    tcp->src_port  = tcp->dest_port;
    tcp->dest_port = len;

    // przepisz numer sekwencyjny jako numer potwierdzenia (+1) + wpisz nasz numer sekwencyjny (losuj)
    tcp->ack_num = htonl(tcp->seq_num);
    tcp->ack_num++;
    tcp->ack_num = htonl(tcp->ack_num);

    srandom(tcp->ack_num);
    tcp->seq_num = htonl(random());

    // SYN+ACK
    tcp->flags = NET_TCP_FLAG_SYN | NET_TCP_FLAG_ACK;

    // rozmiar okna i naglowka
    tcp->window_size = HTONS(5840);
    tcp->hlen = 0x60; /* (24/4) << 4 */

    tcp->options = 0x80050402; // opcje: max. rozmiar segmentu danych: 1408 bajtow

    // zloz pakiety
    len = net_make_ip_packet(ip, NET_IP_TCP, (uint8_t*) tcp, ip->src_addr, 24);
    len = net_make_eth_packet(eth, (uint8_t*) ip, eth->src, len);

    enc28_packet_send((uint8_t*)eth, len);

    //net_dump_eth_packet(eth, len);
}

// -----------------------------------------------------------------------------------------
// potwierdz pakiet od hosta TCP

void net_tcp_acknowledge(tcp_packet* tcp, ethernet_packet* eth)
{
    unsigned int len;
    unsigned long ack_num = tcp->ack_num;
    unsigned long seq_num = tcp->seq_num;

    ip_packet  *ip   = (ip_packet*) (eth->data);

    // przepisz numery portow
    len = tcp->src_port;
    tcp->src_port  = tcp->dest_port;
    tcp->dest_port = len;

    // przepisz numer sekwencyjny jako numer potwierdzenia (+1) + wpisz nasz numer sekwencyjny (losuj)
    tcp->ack_num = htonl(tcp->seq_num) + 1UL;
    tcp->ack_num = htonl(tcp->ack_num);

    tcp->seq_num = ack_num;

    // ACK
    tcp->flags = NET_TCP_FLAG_ACK;

    // rozmiar okna i naglowka
    tcp->window_size = HTONS(5840);
    tcp->hlen = 0x60; /* (24/4) << 4 */

    tcp->options = 0x80050402; // opcje: max. rozmiar segmentu danych: 1408 bajtow

    // zloz pakiety
    len = net_make_ip_packet(ip, NET_IP_TCP, (uint8_t*) tcp, ip->src_addr, 24);
    len = net_make_eth_packet(eth, (uint8_t*) ip, eth->src, len);

    enc28_packet_send((uint8_t*)eth, len);

    //net_dump_eth_packet(eth, len);

    // przywroc dane poprzedniego pakietu - kolejna funkcja wysylajaca sama zamieni numery portow i numery ack i seq
    len = tcp->src_port;
    tcp->src_port  = tcp->dest_port;
    tcp->dest_port = len;

    // numery
    tcp->ack_num = ack_num;
    tcp->seq_num = seq_num;
}

// -----------------------------------------------------------------------------------------
// wpisz dane do pakietu TCP z podanym offset / zwrocony zostanie nowy offset + kontrola bufora

uint16_t net_tcp_write_data_P(tcp_packet* tcp, uint16_t offset, PGM_P data)
{
    uint16_t len = offset;
    unsigned char c;
  	while ( (c = pgm_read_byte(data++)) && (len < ENC28_MAX_FRAMELEN) )
    	tcp->data[len++] = c;

    return len;
}

uint16_t net_tcp_write_data(tcp_packet* tcp, uint16_t offset, unsigned char* data)
{
    uint16_t len = offset;
    unsigned char c;
  	while ( (c = *(data++)) && (len < ENC28_MAX_FRAMELEN) )
    	tcp->data[len++] = c;

    return len;
}


// -----------------------------------------------------------------------------------------
// wyœlij zapytanie ARP o podane IP

void net_arp_ask(ip_addr* ip, ethernet_packet* eth) {

    rs_text_P(PSTR("Pytamy: kto ma IP ")); net_dump_ip(*ip);rs_send('?'); rs_newline();

    arp_packet *arp   = (arp_packet*) (eth->data);

    // wpisz dane pytaj¹cego (czyli nasze)
    memcpy(arp->src_ip,  my_net_config.my_ip,  sizeof(ip_addr));
    memcpy(arp->src_mac, my_net_config.my_mac, sizeof(ip_addr));

    // i treœæ pytania
    memcpy(arp->dest_ip,  ip,   sizeof(ip_addr));
    memset(arp->dest_mac, 0xff, sizeof(mac_addr));

    // rozmiary adresów i typ sieci
    arp->htype = HTONS(0x0001);
    arp->ptype = HTONS(0x0800);
    arp->hlen  = 6;
    arp->plen  = 4;

    // typ pakietu ARP: zapytanie
    arp->opcode = HTONS(NET_ARP_OPCODE_REQUEST);

    // tworz pakiet ethernetowy typiu broadcast
    //mac_addr broadcast;
    //memset((void*) &broadcast, 0xff, sizeof(mac_addr));

    unsigned int len = net_make_eth_packet(eth, (unsigned char*) arp, arp->dest_mac, 28);

    // ustaw typ pakietu na ARP
    eth->eth_type = HTONS(NET_ARP_FRAME);

    // wyœlij zapytanie ARP
    enc28_packet_send((unsigned char*)eth, len);
}

// odpowiedz na zapytanie ARP

void net_arp_response(arp_packet* arp, ethernet_packet* eth)
{
    unsigned char i;

    if (arp->opcode == HTONS(NET_ARP_OPCODE_REQUEST)) {

        // odpowiedŸ na zapytanie ARP o podane IP
        net_dump_ip(arp->src_ip);rs_text_P(PSTR(" pyta: kto ma IP ")); net_dump_ip(arp->dest_ip);rs_send('?');
        rs_newline();

        // sprawdŸ czy to pakiet dla nas (IP)
        if (!net_is_my_ip(arp->dest_ip))
            return;

        // wypelnij pakiet odpowiedzi ARP

        // -> przepisz dane nasze i nadawcy do odpowiedzi
        for (i=0; i<4; i++) {
            arp->dest_ip[i] = arp->src_ip[i];
            arp->src_ip[i] = my_net_config.my_ip[i];
        }

        for (i=0; i<6; i++) {
            arp->dest_mac[i] = arp->src_mac[i];
            arp->src_mac[i] = my_net_config.my_mac[i];
        }

        // typ pakietu ARP: odpowiedŸ
        arp->opcode = HTONS(NET_ARP_OPCODE_REPLY);

        // tworz pakiet ethernetowy
        unsigned int len = net_make_eth_packet(eth, (unsigned char*) arp, arp->dest_mac, 28);

        enc28_packet_send((unsigned char*)eth, len);
    }
    else if (arp->opcode ==  HTONS(NET_ARP_OPCODE_REPLY)) {
        // osb³uga odpowiedzi na wys³ane przez nas zapytanie ARP
        net_dump_ip(arp->src_ip);rs_text_P(PSTR(" odpowiada: mam adres ")); net_dump_mac(arp->src_mac);rs_send('!');
        rs_newline();

        // zapisz adres MAC bramy
        if ( memcmp(arp->src_ip, my_net_config.gate_ip, sizeof(ip_addr)) == 0 ) {
            memcpy(my_net_config.gate_mac, arp->src_mac, sizeof(mac_addr));
        }
    }
}

// -----------------------------------------------------------------------------------------
// DHCP (zapytanie / obsluga odpowiedzi)

void net_dhcp_init(uint8_t* buf)
{
    // zeruj konfiguracje sieciowa
    unsigned char i;

    // czyœæ bufor
    memset((void*) buf, 0, ENC28_MAX_FRAMELEN);

    // zeruj ustawienia bramy i maski sieciowej
    for (i=0; i<4; i++) {
        my_net_config.gate_ip[i] = 0;
        my_net_config.mask[i]    = 0;
    }

    my_net_config.using_dhcp = 0;

    // wyslij pierwsze ¿¹danie
    net_dhcp_send_request(buf, NET_DHCP_MSG_DHCPDISCOVER);
}

void net_dhcp_send_request(uint8_t* buf, uint8_t msg_type)
{
    unsigned char i;
    unsigned int len;

    // wejdz "w glab" kaskady pakietow (ethernet -> IP -> UDP -> DHCP)
    ethernet_packet *eth  = (ethernet_packet*) buf;
    ip_packet       *ip   = (ip_packet*) (eth->data);
    udp_packet      *udp  = (udp_packet*) (ip->data);
    dhcp_packet     *dhcp = (dhcp_packet*) (udp->data);

    // wypelnij pola zadania dla serwera DHCP
    dhcp->op          = NET_DHCP_REQUEST;
    dhcp->htype       = NET_DHCP_HTYPE_ETHERNET;
    dhcp->hlen        = NET_DHCP_HLEN_ETHERNET;

    // wpisujemy puste adresy - dopiero otrzymamy stosowne dane w odpowiedzi od serwera DHCP
    for (i=0; i<4; i++) {
        dhcp->client_addr[i]       = 0; 
        dhcp->client_given_addr[i] = 0;
        dhcp->server_addr[i]       = 0;
        dhcp->gate_addr[i]         = 0;
    }

    // nasz adres MAC
    for (i=0; i<6; i++) {
        dhcp->client_haddr[i] = my_net_config.my_mac[i];
    }

    // NULL terminated string - wstaw NULL
    dhcp->server_name[0] = 0x00;
    dhcp->boot_file[0]   = 0x00;

    dhcp->cookie = 0x63538263;

    dhcp->xid   = ((uint32_t)my_net_config.my_mac[0] << 24) | 
                  ((uint32_t)my_net_config.my_mac[1] << 16) | 
                  ((uint32_t)my_net_config.my_mac[2] << 8) | 
                  my_net_config.my_mac[3];

    dhcp->secs  = 0;
    dhcp->flags = HTONS(1); // flaga UNICAST
    dhcp->hops  = 0;

    //
    // ustaw opcje
    //
    i=0;

    // wpisz opcje "msg_type" (discovery / request)
    dhcp->options[i++] = NET_DHCP_OPTION_DHCPMSGTYPE;
    dhcp->options[i++] = 1;
    dhcp->options[i++] = msg_type;

    // wyslij potwierdzenie przyjecia proponowanego IP (REQUEST)
    if (msg_type == NET_DHCP_MSG_DHCPREQUEST) {
        dhcp->options[i++] = NET_DHCP_OPTION_REQUESTEDIP;
        dhcp->options[i++] = 4;
        dhcp->options[i++] = my_net_config.my_ip[0];
        dhcp->options[i++] = my_net_config.my_ip[1];
        dhcp->options[i++] = my_net_config.my_ip[2];
        dhcp->options[i++] = my_net_config.my_ip[3];
    }
    
    // zakoncz opcje
    dhcp->options[i++] = NET_DHCP_OPTION_END;

    // wyslij zapytanie DHCP (broadcast)
    ip_addr  broadcast_ip = {0xff, 0xff, 0xff, 0xff};
    mac_addr broadcast_mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    len = net_make_udp_packet(udp, NET_DHCP_UDP_CLIENT_PORT, NET_DHCP_UDP_SERVER_PORT, (uint8_t*) dhcp, 240 + i);
    len = net_make_ip_packet(ip, NET_IP_UDP, (uint8_t*) udp, broadcast_ip, len);
    len = net_make_eth_packet(eth, (uint8_t*) ip, broadcast_mac, len);

    // ustaw typ pakietu na IPv4
    eth->eth_type = HTONS(NET_IP4_FRAME);

    //net_dump_eth_packet(eth, len);

    rs_text_P(PSTR("DHCP: wyslano pakiet ")); rs_text_P((msg_type == NET_DHCP_MSG_DHCPDISCOVER) ? PSTR("discovery") : PSTR("request")); rs_newline();

    // wyslij pakiet
    enc28_packet_send((uint8_t*)eth, len);

    return;
}

unsigned char net_dhcp_handle_answer(dhcp_packet* dhcp, ethernet_packet* eth)
{
    unsigned char i;
    uint8_t buf[4];

    if (dhcp->op != NET_DHCP_REPLY)
        return 0;

    // pobierz typ wiadomosci DHCP
    net_dhcp_option_get(dhcp->options, NET_DHCP_OPTION_DHCPMSGTYPE, 1, &i);

    switch (i)
    {
        case NET_DHCP_MSG_DHCPOFFER:
            // oferta od DHCP z adresami
            rs_newline();
            rs_text_P(PSTR("DHCP: oferta ")); net_dump_ip(dhcp->client_given_addr);
            rs_text_P(PSTR(" (od ")); net_dump_ip(dhcp->server_addr);rs_send(')');
            rs_newline();

            // wyslij potwierdzenie i zadanie wiekszej ilosci informacji
            for (i=0; i<4; i++) my_net_config.my_ip[i] = dhcp->client_given_addr[i];

            net_dhcp_send_request(net_packet, NET_DHCP_MSG_DHCPREQUEST);

            return 1;

        case NET_DHCP_MSG_DHCPACK:
            // przyjecie oferty od DHCP -> dalsze informacje i koniec konfiguracji warstwy sieciowej

            // maska sieciowa
            net_dhcp_option_get(dhcp->options, NET_DHCP_OPTION_NETMASK, 4, &buf);
            for (i=0; i<4; i++) my_net_config.mask[i] = buf[i];

            // brama (IP/MAC)
            net_dhcp_option_get(dhcp->options, NET_DHCP_OPTION_ROUTER, 4, &buf);
            for (i=0; i<4; i++) my_net_config.gate_ip[i]  = buf[i];
            for (i=0; i<6; i++) my_net_config.gate_mac[i] = eth->src[i];
            
            // przydzielone IP
            for (i=0; i<4; i++) my_net_config.my_ip[i] = dhcp->client_given_addr[i];

            // tak, uzywamy od teraz konfiguracji z DHCP
            my_net_config.using_dhcp = 1;

            // pokaz informacje o uzyskanej konfiguracji
            rs_newline();
            rs_text_P(PSTR("DHCP: gotowe!")); rs_newline();
            rs_text_P(PSTR("IP:    ")); net_dump_ip(my_net_config.my_ip); rs_newline();

            rs_text_P(PSTR("Brama: ")); net_dump_ip(my_net_config.gate_ip);
                rs_send('/');net_dump_mac(my_net_config.gate_mac);rs_newline();
            
            rs_text_P(PSTR("Maska: ")); net_dump_ip(my_net_config.mask); rs_newline(); rs_newline();

            return 2;
    }

    return 0;
}

uint8_t  net_dhcp_option_get(uint8_t* options, uint8_t optcode, uint8_t optlen, void* optvalptr)
{
	uint8_t i;

	// parse for desired option
	for (;;)
	{
		// skip pad characters
		if(*options == NET_DHCP_OPTION_PAD)
			options++;
		// break if end reached
		else if(*options == NET_DHCP_OPTION_END)
			break;
		// check for desired option
		else if(*options == optcode)
		{
			// found desired option
			// limit size to actual option length
			//optlen = MIN(optlen, *(options+1));
            optlen = (optlen < *(options+1) ) ? optlen : *(options+1) ;

			
			// copy contents of option
			for(i=0; i<optlen; i++)
				*(((uint8_t*)optvalptr)+i) = *(options+i+2);
			// return length of option
			return *(options+1);
		}
		else
		{
			// skip to next option
			options++;
			options+=*options;
			options++;
		}
	}
	// failed to find desired option
	return 0;
}


// -----------------------------------------------------------------------------------------
// odpowiedz na PING

void net_icmp_response(icmp_packet* icmp, ethernet_packet* eth)
{
    // odpowiedz tylko na ECHO REQUEST
    if (icmp->type != NET_ICMP_TYPE_ECHO_REQUEST)
        return;

    // pobierz rozmiar odebranego pakietu ICMP z pakietu IP (zwracamy te same dane...)
    ip_packet *ip = (ip_packet*) eth->data;

    unsigned int len = HTONS(ip->length)-20;

    // odpowiedz na PING
    icmp->type = NET_ICMP_TYPE_ECHO_REPLY;
    
    // suma kontrolna
    icmp->checksum = 0;
    icmp->checksum = net_checksum((uint8_t*)icmp, len, 0);

    // zloz pakiet IP
    len = net_make_ip_packet( ip, NET_IP_ICMP, (unsigned char*) icmp, ip->src_addr, len);

    // zloz pakiet ethernetowy
    len = net_make_eth_packet(eth, (unsigned char*) eth->data, eth->src, len);

    enc28_packet_send((unsigned char*)eth, len);

    return;
}

// -----------------------------------------------------------------------------------------
// NTP
void net_ntp_get_time(uint8_t* buf)
{
    unsigned int len;

    // adres serwera NTP
    //ip_addr net_ntp_server = {198, 123, 30, 132}; // ntp.nasa.gov 198.123.30.132
    ip_addr net_ntp_server = {212, 244, 36, 227}; // tempus1.gum.gov.pl 212.244.36.227
    //ip_addr net_ntp_server = {212, 244, 36, 228}; // tempus2.gum.gov.pl 212.244.36.228
    //ip_addr net_ntp_server = {149, 156, 70, 60}; // 0.pl.pool.ntp.org 149.156.70.60
    //ip_addr net_ntp_server = {150, 254, 183, 15}; // vega.cbk.poznan.pl 150.254.183.15

    // wejdz "w glab" kaskady pakietow (ethernet -> IP -> UDP -> DHCP)
    ethernet_packet *eth  = (ethernet_packet*) buf;
    ip_packet       *ip   = (ip_packet*) (eth->data);
    udp_packet      *udp  = (udp_packet*) (ip->data);
    ntp_packet      *ntp  = (ntp_packet*) (udp->data);

    // kilka informacji
    ntp->flags = 0xe3; // 11 100 011 - alarm / ntp v4 / klient (3)
    ntp->type  = 0;
    ntp->precision = HTONS(0x04fa); //  16 sekund / 0.015625 s
    ntp->est_error = htonl(0x00010001); // blad: 1 sekunda

    // puste pola
    ntp->est_drift_rate = 0L;
    ntp->ref_clock_id = 0L;
    ntp->ref_timestamp = 0L;
    ntp->orig_timestamp = 0L;
    ntp->recv_timestamp = 0L;
    ntp->tran_timestamp = 0L;
    ntp->tran_timestamp_frac = 0L;

    // wyslij pakiet NTP (opakowujac w kolejne pakiety UDP->IP->ethernet)
    len = net_make_udp_packet(udp, NET_NTP_ANSWER, NET_NTP_PORT, (uint8_t*) ntp, 48);
    len = net_make_ip_packet(ip, NET_IP_UDP, (uint8_t*) udp, net_ntp_server, len);
    len = net_make_eth_packet(eth, (uint8_t*) ip, my_net_config.gate_mac, len); // wysylamy na MAC bramy

    // ustaw typ pakietu na IPv4
    eth->eth_type = HTONS(NET_IP4_FRAME);

    //net_dump_eth_packet(eth, len);

    // wyslij pakiet
    enc28_packet_send((uint8_t*)eth, len);
}

void net_ntp_handle_answer(ntp_packet* ntp, void (*func)(time_t*))
{
    time_t   time;
    uint32_t timestamp = htonl(ntp->tran_timestamp);

    //rs_dump((unsigned char*)ntp,  sizeof(ntp_packet));

    if (timestamp & 0x80000000UL) {
        // czas 32-bitowy "przekreci" licznik: 7-Feb-2036 @ 06:28:16 UTC
        timestamp += 2085978496UL;
    }
    else {
        // uwzglednij przesuniecie 1-1-1900 <> 1-1-1970
        timestamp -= NET_NTP_GETTIMEOFDAY_TO_NTP_OFFSET;
    }

    // strefa czasowa (wzgledem UTC)
    timestamp -= (UTC_OFFSET*3600);

    // konwersja timestamp -> struktura czasu
    gmtime(timestamp, &time);

    rs_text_P(PSTR("NTP: 20")); rs_int2(time.tm_year); rs_send('-'); rs_int2(time.tm_mon); rs_send('-'); rs_int2(time.tm_mday); rs_send(' ');
        rs_int2(time.tm_hour); rs_send(':'); rs_int2(time.tm_min); rs_send(':'); rs_int2(time.tm_sec); rs_newline();

    // wywolaj funkcje ustawiajaca czas
    func(&time);
}


uint8_t monthlen(uint8_t isleapyear,uint8_t month){
	if(month==1)
		return(28+isleapyear);
	
    if (month>6)
		month--;
	
    if (month%2==1)
		return(30);
	
    return(31);
} 

void gmtime(uint32_t timestamp,time_t* time)
{
	uint32_t dayclock;
    uint16_t dayno;
    uint16_t tm_year = NET_NTP_EPOCH_YEAR;

	dayclock = timestamp % NET_NTP_SECS_DAY;
	dayno = timestamp / NET_NTP_SECS_DAY;

	time->tm_sec = dayclock % 60UL;
	time->tm_min = (dayclock % 3600UL) / 60;
	time->tm_hour = dayclock / 3600UL;
	time->tm_wday = (dayno + 4) % 7;	/* day 0 was a thursday */
	
    while (dayno >= NET_NTP_YEARSIZE(tm_year)) {
		dayno -= NET_NTP_YEARSIZE(tm_year);
		tm_year++;
	}

    time->tm_year = tm_year - 2000;

	time->tm_mon = 0;
	while (dayno >= monthlen(NET_NTP_LEAPYEAR(tm_year),time->tm_mon)) {
		dayno -= monthlen(NET_NTP_LEAPYEAR(tm_year),time->tm_mon);
		time->tm_mon++;
	}

    time->tm_mon++;

    time->tm_mday = dayno+1;

    return;
}

// -----------------------------------------------------------------------------------------
// mikro-stos TCP/IP
unsigned char net_stack(ethernet_packet *eth_packet)
{
    ip_packet*    ip;
    unsigned int  port;
    unsigned char flags;

    // zwiêksz licznik odebranych pakietów
    my_net_config.pktcnt++;

    switch(HTONS(eth_packet->eth_type)) {
        // --------------------------------------------------------------------------------
        case NET_ARP_FRAME:
            // stworz odpowiedz na zapytanie ARP
            net_arp_response((arp_packet*) eth_packet->data, eth_packet);
            break;

        // --------------------------------------------------------------------------------
        case NET_IP4_FRAME:
            // czy pakiet IP jest na pewno dla nas? (sprawdz IP)
            ip = (ip_packet*) (eth_packet->data);

            // parsuj pakiet IP w zaleznosci od jego typu
            if ( net_is_my_ip(ip->dest_addr) ) {
                // dane pakietu IP (przesuniecie wylicz z wartosci pola IHL naglowka razy 4)
                unsigned char* ip_data = (((unsigned char*) ip) + ((ip->ver_ihl & 0x0f) * 4) );

                switch ( ip->proto )
                {
                    case NET_IP_ICMP:
                        net_icmp_response( (icmp_packet*) ip_data, eth_packet);

                        // odpowiedz na PINGa
                        net_dump_ip(ip->dest_addr); rs_text_P(PSTR(": ping? pong!")); rs_newline();
                        break;

                    case NET_IP_UDP:
                        // pakiet UDP -> sprawdz numer portu
                        port = ((udp_packet*) ip_data)->dest_port;
                        port = HTONS(port);

                        //rs_text_P(PSTR("UDP od ")); net_dump_ip(ip->src_addr);rs_send(':'); rs_int(port);rs_newline();

                        switch (port)
                        {
                            case NET_DHCP_UDP_CLIENT_PORT:
                                // odpowiedz od DHCP
                                net_dhcp_handle_answer( (dhcp_packet*) (((udp_packet*) ip_data)->data), eth_packet);
                                break;

                            case NET_NTP_ANSWER:
                                // aktualizacja czasu (NTP)
                                net_ntp_handle_answer( (ntp_packet*) (((udp_packet*) ip_data)->data), ds1306_time_set);
                                break;

                            default:
                                return NET_STACK_UNHANDLED;
                                break;
                        }

                        break;

                    case NET_IP_TCP:
                        // pakiet TCP
                        port = ((tcp_packet*) ip_data)->dest_port;
                        port = HTONS(port);

                        //
                        // Flagi dla przykladowej sesji TCP:  > klient  < enc28j60
                        //  > SYN
                        //  < SYN+ACK
                        //  > ACK
                        //  > ACK+PUSH (tresc zadania)
                        //  < ACK
                        //  < ACK+FIN+PUSH (odpowiedz)
                        //  > FIN+ACK
                        //  < ACK
                        //

                        // flagi (SYN/ACK)
                        flags = ((tcp_packet*) ip_data)->flags;

                        // odpowiedz na pakiet z flaga SYN -> odeslij SYN+ACK
                        if ( flags & NET_TCP_FLAG_SYN ) {
                            net_tcp_synchronize( (tcp_packet*) ip_data, eth_packet);
                        } 

                        // potwierdzenie zakonczenia sesji
                        else if ( (flags & NET_TCP_FLAG_FIN) && (flags & NET_TCP_FLAG_ACK) ) {
                            net_tcp_acknowledge( (tcp_packet*) ip_data, eth_packet);
                        }

                        // odpowiedz na pakiet ACK+PUSH (zadanie na konkretny port, np. HTTP)
                        else if ( (flags & NET_TCP_FLAG_PUSH) && (flags & NET_TCP_FLAG_ACK) )
                            return NET_STACK_UNHANDLED;
                        
                        break;
                }
            }
            break;
    }

    return NET_STACK_HANDLED;

}

