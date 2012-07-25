#ifndef _NET_H
#define _NET_H

#include "../telemetry.h"

// definicje typów
typedef uint8_t mac_addr[6];    // adres sprzêtowy MAC xx:xx:xx:xx:xx:xx
typedef uint8_t ip_addr[4];     // adres IP xx.xx.xx.xx

// konfiguracja: MAC / IP
typedef struct
{
    mac_addr my_mac;
    ip_addr  my_ip;
    mac_addr gate_mac;
    ip_addr  gate_ip;
    ip_addr  mask;
    unsigned int pktcnt;
    uint8_t  using_dhcp;
    uint8_t  using_fullduplex;
} net_config;

net_config my_net_config;

// licznik ID pakietow IP
volatile uint16_t net_ip_packet_id;

// -----------------------------------------------------------------------------------------
// ramka Ethernet
typedef struct
{
    mac_addr dest;
    mac_addr src;
    uint16_t eth_type;
    uint8_t  data[];
} ethernet_packet; /* 14 */

// typy danych przesylanych przez ramke Ethernetowa
// @see http://en.wikipedia.org/wiki/EtherType
#define NET_IP4_FRAME 0x0800
#define NET_ARP_FRAME 0x0806

// -----------------------------------------------------------------------------------------
// ARP (Address Resolution Protocol) dla sieci Ethernet
// HLEN = 6 PLEN = 4
typedef struct
{
    uint16_t    htype;
    uint16_t    ptype;
    uint8_t     hlen;
    uint8_t     plen;
    uint16_t    opcode;
    uint8_t     src_mac[6];
    uint8_t     src_ip[4];
    uint8_t     dest_mac[6];
    uint8_t     dest_ip[4];
} arp_packet; /* 28 */

// typy zapytan ARP
#define NET_ARP_OPCODE_REQUEST	0x0001
#define NET_ARP_OPCODE_REPLY	0x0002

// -----------------------------------------------------------------------------------------
// IP
typedef struct
{
    uint8_t  ver_ihl;  // zwykle: 0x45 (IPv4, dlugosc naglowka 5 * 4 bajty)
    uint8_t  tos;
    uint16_t length;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t  ttl;
    uint8_t  proto;
    uint16_t checksum;
    ip_addr  src_addr;
    ip_addr  dest_addr;
    uint8_t  data[];
} ip_packet;

// http://tools.ietf.org/html/rfc1700
#define NET_IP_ICMP 0x01
#define NET_IP_TCP  0x06
#define NET_IP_UDP  0x11

// -----------------------------------------------------------------------------------------
// TCP
typedef struct
{
    uint16_t    src_port;
    uint16_t    dest_port;
    uint32_t    seq_num;
    uint32_t    ack_num;
    uint8_t     hlen;
    uint8_t     flags;
    uint16_t    window_size;
    uint16_t    checksum;
    uint16_t    priority;
    uint32_t    options;
    uint8_t     data[];
} tcp_packet; /* 20 bez opcji */

// flagi TCP
#define NET_TCP_FLAG_FIN    1
#define NET_TCP_FLAG_SYN    2
#define NET_TCP_FLAG_RESET  4
#define NET_TCP_FLAG_PUSH   8
#define NET_TCP_FLAG_ACK    16
#define NET_TCP_FLAG_URGENT 32
#define NET_TCP_FLAG_ECN    64
#define NET_TCP_FLAG_CWR    128


// -----------------------------------------------------------------------------------------
// UDP
typedef struct
{
    uint16_t    src_port;
    uint16_t    dest_port;
    uint16_t    length;
    uint16_t    checksum;
    uint8_t     data[];
} udp_packet;


// -----------------------------------------------------------------------------------------
// ICMP
typedef struct
{
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
    uint8_t  data[];
} icmp_packet;

#define NET_ICMP_TYPE_ECHO_REPLY    0x00
#define NET_ICMP_TYPE_ECHO_REQUEST  0x08


// -----------------------------------------------------------------------------------------
// DHCP
typedef struct
{
    uint8_t  op;
    uint8_t  htype;
    uint8_t  hlen;
    uint8_t  hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    ip_addr  client_addr;
    ip_addr  client_given_addr;
    ip_addr  server_addr;
    ip_addr  gate_addr;
    uint8_t  client_haddr[8];
    uint8_t  fill[8];           /* */
    uint8_t  server_name[64];
    uint8_t  boot_file[128];
    uint32_t cookie;
    uint8_t  options[];
} dhcp_packet; /* 240 (bez opcji) */

#define NET_DHCP_REQUEST	        1
#define NET_DHCP_REPLY		        2

#define NET_DHCP_HTYPE_ETHERNET	    1
#define NET_DHCP_HLEN_ETHERNET	    6

#define NET_DHCP_UDP_SERVER_PORT	67
#define NET_DHCP_UDP_CLIENT_PORT	68

#define NET_DHCP_OPTION_PAD         0
#define NET_DHCP_OPTION_NETMASK     1
#define NET_DHCP_OPTION_ROUTER      3
#define NET_DHCP_OPTION_HOSTNAME    12
#define NET_DHCP_OPTION_DOMAINNAME  15
#define NET_DHCP_OPTION_REQUESTEDIP 50
#define NET_DHCP_OPTION_LEASETIME   51
#define NET_DHCP_OPTION_DHCPMSGTYPE	53
#define NET_DHCP_OPTION_END     	255

#define NET_DHCP_MSG_DHCPDISCOVER	1
#define NET_DHCP_MSG_DHCPOFFER  	2
#define NET_DHCP_MSG_DHCPREQUEST	3
#define NET_DHCP_MSG_DHCPACK		5


// -----------------------------------------------------------------------------------------
// NTP       @see http://tools.ietf.org/rfc/rfc958.txt
typedef struct
{
    uint8_t  flags; // leap indicator / version / mode
    uint8_t  type;
    uint16_t precision;
    uint32_t est_error;
    uint32_t est_drift_rate;
    uint32_t ref_clock_id;
    uint64_t ref_timestamp;
    uint64_t orig_timestamp;
    uint64_t recv_timestamp;
    uint32_t tran_timestamp;
    uint32_t tran_timestamp_frac;
} ntp_packet; /* 48 */

// -----------------------------------------------------------------------------------------
// zamiana kolejnosci bajtow dla wartosci 16/32-bitowych
#define HTONS(s)		((s<<8) | (s>>8))
uint16_t htons(uint16_t val);
uint32_t htonl(uint32_t val);

// -----------------------------------------------------------------------------------------
// funkcje narzedziowe: sprawdzenie zgodnosci podanego IP/MAC, obliczenie sumy kontrolnej dla pakietow IP (IP/UDP)
uint8_t net_is_my_ip(uint8_t*);
uint8_t net_is_my_mac(uint8_t*);
//uint16_t net_checksum(uint8_t*, uint16_t,uint8_t);
uint16_t net_checksum(void*, uint16_t, uint8_t);

// -----------------------------------------------------------------------------------------
// Funkcje obslugi pakietow

// obsluz podany pakiet Ethernetowy
ethernet_packet* net_handle_eth_packet(uint8_t*, uint8_t);

// stworz pakiet Ethernetowy
uint16_t net_make_eth_packet(ethernet_packet*, uint8_t*, uint8_t*, uint16_t);

// stworz pakiet IP (wypelnia pola TTL, opcje, adresy, liczy sume kontrolna)
uint16_t net_make_ip_packet(ip_packet*, uint8_t, uint8_t*, uint8_t*, uint16_t);

// stworz pakiet UDP (wypelnia pola portow, liczy sume kontrolna)
uint16_t net_make_udp_packet(udp_packet*, uint16_t, uint16_t, uint8_t*, uint16_t);

// stworz pakiet TCP (wypelnia pola portow, liczy sume kontrolna)
uint16_t net_make_tcp_packet(tcp_packet*, uint8_t, uint16_t, uint16_t, uint8_t*, uint16_t);

// -----------------------------------------------------------------------------------------
// TCP

// synchronizuj z hostem
void net_tcp_synchronize(tcp_packet*, ethernet_packet*);

// wyslij potwierdzenie otrzymania pakietu
void net_tcp_acknowledge(tcp_packet*, ethernet_packet*);

// wpisz dane do pakietu TCP
uint16_t net_tcp_write_data(tcp_packet*, uint16_t, unsigned char*);
uint16_t net_tcp_write_data_P(tcp_packet*, uint16_t, PGM_P);

// -----------------------------------------------------------------------------------------
// ARP

// zapytaj o adres MAC podanego adresu IP
void net_arp_ask(ip_addr* ip, ethernet_packet* eth);

// odpowiedz na zadanie ARP (zwroc swoj adres MAC i IP)
void net_arp_response(arp_packet*, ethernet_packet*);

// -----------------------------------------------------------------------------------------
// DHCP

// rozpoczyna producedure uzyskania adresu IP z serwera DHCP
void net_dhcp_init(uint8_t*);

// wyslij zapytanie do serwera DHCP
void net_dhcp_send_request(uint8_t*, uint8_t);

// obsluz odpowiedz z serwera DHCP
unsigned char net_dhcp_handle_answer(dhcp_packet*, ethernet_packet*);

// pobranie / odczyt opcji pakietu DHCP
uint8_t  net_dhcp_option_get(uint8_t*, uint8_t, uint8_t, void*);

// -----------------------------------------------------------------------------------------
// ICMP

// odpowiedz na ICMP (odpowiedz na PING)
void net_icmp_response(icmp_packet*, ethernet_packet*);

// -----------------------------------------------------------------------------------------
// NTP

// wyslij zadanie pobrania czasu z serwera NTP
void net_ntp_get_time(uint8_t*);

// obsluz pakiet odpowiedzi z serwera NTP (jako drugi parametr funkcja otrzymujaca wskaznik do struktury *time)
void net_ntp_handle_answer(ntp_packet*, void (*func)(time_t*));

#define NET_NTP_PORT      123   // port serwera NTP
#define NET_NTP_ANSWER  10123   // port klienta NTP

// sekundy od 1-1-1900 do 1-1-1970
#define NET_NTP_GETTIMEOFDAY_TO_NTP_OFFSET 2208988800UL

// EPOCH = Jan 1 1970 00:00:00
#define	NET_NTP_EPOCH_YEAR	1970
//(24L * 60L * 60L)
#define	NET_NTP_SECS_DAY	86400UL  
#define	NET_NTP_LEAPYEAR(year)	(!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define	NET_NTP_YEARSIZE(year)	(NET_NTP_LEAPYEAR(year) ? 366 : 365)

void gmtime(uint32_t,time_t*); 

// -----------------------------------------------------------------------------------------
// mikro-stos TCP/IP
//
// funkcja obsluguje "znane" pakiety (NTP/DHCP/ARP/ICMP)
unsigned char net_stack(ethernet_packet*);

// pakiet zosta³ / niezosta³ obs³uzony
#define NET_STACK_HANDLED   1
#define NET_STACK_UNHANDLED 0

// -----------------------------------------------------------------------------------------
// debug

// zrzut pakietu ethernetowego
void net_dump_eth_packet(ethernet_packet*, uint16_t);
void net_dump_ip(ip_addr);
void net_dump_mac(mac_addr);

// zrzut informacji o konfiguracji stosu
void net_dump_info();

#endif
