#ifndef _ENC28_H
#define _ENC28_H

#include "../telemetry.h"

//#define ENC28_SS_PORT       PORTB
//#define ENC28_SS_PIN        3

// maski
#define ENC28_MASK_ADDRESS  0x1f         // maska dla instrukcji (opcode - 3 MSB / adres - 5 kolejnych bajtow)
#define ENC28_MASK_BANK     (0x40|0x20)  // maska dla numeru banku "ukrytego" w ID rejestru
#define ENC28_MASK_MII      0x80         // maska dla rejestrow adresowanych poprzez MIIM (rejestry MAC/MIII)

// instrukcje (opcodes)
#define ENC28_OPCODE_RCR    0b00000000    // read control register
#define ENC28_OPCODE_RBM    0b00111010    // read buffer memory
#define ENC28_OPCODE_WCR    0b01000000    // write control register
#define ENC28_OPCODE_WBM    0b01111010    // write buffer memory
#define ENC28_OPCODE_BFS    0b10000000    // bit field set
#define ENC28_OPCODE_BFC    0b10100000    // bit field clear
#define ENC28_OPCODE_SRC    0b11111111    // system reset command (soft reset)

//
// rejestry
//

// wspolne
#define ENC28_EIE               0x1B
#define ENC28_EIR               0x1C
#define ENC28_ESTAT             0x1D
#define ENC28_ECON2             0x1E
#define ENC28_ECON1             0x1F

// bank 0
#define ENC28_ERDPTL            0x00
#define ENC28_ERDPTH            0x01
#define ENC28_EWRPTL            0x02
#define ENC28_EWRPTH            0x03
#define ENC28_ETXSTL            0x04
#define ENC28_ETXSTH            0x05
#define ENC28_ETXNDL            0x06
#define ENC28_ETXNDH            0x07
#define ENC28_ERXSTL            0x08
#define ENC28_ERXSTH            0x09
#define ENC28_ERXNDL            0x0A
#define ENC28_ERXNDH            0x0B
#define ENC28_ERXRDPTL          0x0C
#define ENC28_ERXRDPTH          0x0D
#define ENC28_ERXWRPTL          0x0E
#define ENC28_ERXWRPTH          0x0F
#define ENC28_EDMASTL           0x10
#define ENC28_EDMASTH           0x11
#define ENC28_EDMANDL           0x12
#define ENC28_EDMANDH           0x13
#define ENC28_EDMADSTL          0x14
#define ENC28_EDMADSTH          0x15
#define ENC28_EDMACSL           0x16
#define ENC28_EDMACSH           0x17

// bank 1
#define ENC28_EHT0             (0x00|0x20)
#define ENC28_EHT1             (0x01|0x20)
#define ENC28_EHT2             (0x02|0x20)
#define ENC28_EHT3             (0x03|0x20)
#define ENC28_EHT4             (0x04|0x20)
#define ENC28_EHT5             (0x05|0x20)
#define ENC28_EHT6             (0x06|0x20)
#define ENC28_EHT7             (0x07|0x20)
#define ENC28_EPMM0            (0x08|0x20)
#define ENC28_EPMM1            (0x09|0x20)
#define ENC28_EPMM2            (0x0A|0x20)
#define ENC28_EPMM3            (0x0B|0x20)
#define ENC28_EPMM4            (0x0C|0x20)
#define ENC28_EPMM5            (0x0D|0x20)
#define ENC28_EPMM6            (0x0E|0x20)
#define ENC28_EPMM7            (0x0F|0x20)
#define ENC28_EPMCSL           (0x10|0x20)
#define ENC28_EPMCSH           (0x11|0x20)
#define ENC28_EPMOL            (0x14|0x20)
#define ENC28_EPMOH            (0x15|0x20)
#define ENC28_EWOLIE           (0x16|0x20)
#define ENC28_EWOLIR           (0x17|0x20)
#define ENC28_ERXFCON          (0x18|0x20)
#define ENC28_EPKTCNT          (0x19|0x20) 

// bank 2
#define ENC28_MACON1           (0x00|0x40|0x80)
#define ENC28_MACON2           (0x01|0x40|0x80)
#define ENC28_MACON3           (0x02|0x40|0x80)
#define ENC28_MACON4           (0x03|0x40|0x80)
#define ENC28_MABBIPG          (0x04|0x40|0x80)
#define ENC28_MAIPGL           (0x06|0x40|0x80)
#define ENC28_MAIPGH           (0x07|0x40|0x80)
#define ENC28_MACLCON1         (0x08|0x40|0x80)
#define ENC28_MACLCON2         (0x09|0x40|0x80)
#define ENC28_MAMXFLL          (0x0A|0x40|0x80)
#define ENC28_MAMXFLH          (0x0B|0x40|0x80)
#define ENC28_MAPHSUP          (0x0D|0x40|0x80)
#define ENC28_MICON            (0x11|0x40|0x80)
#define ENC28_MICMD            (0x12|0x40|0x80)
#define ENC28_MIREGADR         (0x14|0x40|0x80)
#define ENC28_MIWRL            (0x16|0x40|0x80)
#define ENC28_MIWRH            (0x17|0x40|0x80)
#define ENC28_MIRDL            (0x18|0x40|0x80)
#define ENC28_MIRDH            (0x19|0x40|0x80) 

// bank 3
#define ENC28_MAADR1           (0x00|0x60|0x80)
#define ENC28_MAADR0           (0x01|0x60|0x80)
#define ENC28_MAADR3           (0x02|0x60|0x80)
#define ENC28_MAADR2           (0x03|0x60|0x80)
#define ENC28_MAADR5           (0x04|0x60|0x80)
#define ENC28_MAADR4           (0x05|0x60|0x80)
#define ENC28_EBSTSD           (0x06|0x60)
#define ENC28_EBSTCON          (0x07|0x60)
#define ENC28_EBSTCSL          (0x08|0x60)
#define ENC28_EBSTCSH          (0x09|0x60)
#define ENC28_MISTAT           (0x0A|0x60|0x80)
#define ENC28_EREVID           (0x12|0x60)
#define ENC28_ECOCON           (0x15|0x60)
#define ENC28_EFLOCON          (0x17|0x60)
#define ENC28_EPAUSL           (0x18|0x60)
#define ENC28_EPAUSH           (0x19|0x60) 

// rejestr PHY
#define ENC28_PHCON1           0x00
#define ENC28_PHSTAT1          0x01
#define ENC28_PHHID1           0x02
#define ENC28_PHHID2           0x03
#define ENC28_PHCON2           0x10
#define ENC28_PHSTAT2          0x11
#define ENC28_PHIE             0x12
#define ENC28_PHIR             0x13
#define ENC28_PHLCON           0x14


//
// definicje bitow w rejestrach ENC28J60
//

// rejestr ERXFCON
#define ENC28_ERXFCON_UCEN     0x80
#define ENC28_ERXFCON_ANDOR    0x40
#define ENC28_ERXFCON_CRCEN    0x20
#define ENC28_ERXFCON_PMEN     0x10
#define ENC28_ERXFCON_MPEN     0x08
#define ENC28_ERXFCON_HTEN     0x04
#define ENC28_ERXFCON_MCEN     0x02
#define ENC28_ERXFCON_BCEN     0x01
// rejestr EIE
#define ENC28_EIE_INTIE        0x80
#define ENC28_EIE_PKTIE        0x40
#define ENC28_EIE_DMAIE        0x20
#define ENC28_EIE_LINKIE       0x10
#define ENC28_EIE_TXIE         0x08
#define ENC28_EIE_WOLIE        0x04
#define ENC28_EIE_TXERIE       0x02
#define ENC28_EIE_RXERIE       0x01
// rejestr EIR
#define ENC28_EIR_PKTIF        0x40
#define ENC28_EIR_DMAIF        0x20
#define ENC28_EIR_LINKIF       0x10
#define ENC28_EIR_TXIF         0x08
#define ENC28_EIR_WOLIF        0x04
#define ENC28_EIR_TXERIF       0x02
#define ENC28_EIR_RXERIF       0x01
// rejestr ESTAT
#define ENC28_ESTAT_INT        0x80
#define ENC28_ESTAT_LATECOL    0x10
#define ENC28_ESTAT_RXBUSY     0x04
#define ENC28_ESTAT_TXABRT     0x02
#define ENC28_ESTAT_CLKRDY     0x01
// rejestr ECON2
#define ENC28_ECON2_AUTOINC    0x80
#define ENC28_ECON2_PKTDEC     0x40
#define ENC28_ECON2_PWRSV      0x20
#define ENC28_ECON2_VRPS       0x08
// rejestr ECON1
#define ENC28_ECON1_TXRST      0x80
#define ENC28_ECON1_RXRST      0x40
#define ENC28_ECON1_DMAST      0x20
#define ENC28_ECON1_CSUMEN     0x10
#define ENC28_ECON1_TXRTS      0x08
#define ENC28_ECON1_RXEN       0x04
#define ENC28_ECON1_BSEL1      0x02
#define ENC28_ECON1_BSEL0      0x01
// rejestr MACON1
#define ENC28_MACON1_LOOPBK    0x10
#define ENC28_MACON1_TXPAUS    0x08
#define ENC28_MACON1_RXPAUS    0x04
#define ENC28_MACON1_PASSALL   0x02
#define ENC28_MACON1_MARXEN    0x01
// rejestr MACON2
#define ENC28_MACON2_MARST     0x80
#define ENC28_MACON2_RNDRST    0x40
#define ENC28_MACON2_MARXRST   0x08
#define ENC28_MACON2_RFUNRST   0x04
#define ENC28_MACON2_MATXRST   0x02
#define ENC28_MACON2_TFUNRST   0x01
// rejestr MACON3
#define ENC28_MACON3_PADCFG2   0x80
#define ENC28_MACON3_PADCFG1   0x40
#define ENC28_MACON3_PADCFG0   0x20
#define ENC28_MACON3_TXCRCEN   0x10
#define ENC28_MACON3_PHDRLEN   0x08
#define ENC28_MACON3_HFRMLEN   0x04
#define ENC28_MACON3_FRMLNEN   0x02
#define ENC28_MACON3_FULDPX    0x01
// rejestr MICMD
#define ENC28_MICMD_MIISCAN    0x02
#define ENC28_MICMD_MIIRD      0x01
// rejestr MISTAT
#define ENC28_MISTAT_NVALID    0x04
#define ENC28_MISTAT_SCAN      0x02
#define ENC28_MISTAT_BUSY      0x01
// rejestr PHY PHCON1
#define ENC28_PHCON1_PRST      0x8000
#define ENC28_PHCON1_PLOOPBK   0x4000
#define ENC28_PHCON1_PPWRSV    0x0800
#define ENC28_PHCON1_PDPXMD    0x0100
// rejestr PHY PHSTAT1
#define ENC28_PHSTAT1_PFDPX    0x1000
#define ENC28_PHSTAT1_PHDPX    0x0800
#define ENC28_PHSTAT1_LLSTAT   0x0004
#define ENC28_PHSTAT1_JBSTAT   0x0002
// rejestr PHY PHCON2
#define ENC28_PHCON2_FRCLINK   0x4000
#define ENC28_PHCON2_TXDIS     0x2000
#define ENC28_PHCON2_JABBER    0x0400
#define ENC28_PHCON2_HDLDIS    0x0100

// kontrola pakietow
#define ENC_PKTCTRL_PHUGEEN  0x08
#define ENC_PKTCTRL_PPADEN   0x04
#define ENC_PKTCTRL_PCRCEN   0x02
#define ENC_PKTCTRL_POVERRIDE 0x01 


// poczatek bufora odbiorczego
#define ENC28_RXSTART_INIT      0x0
// koniec bufora odbiorczego
#define ENC28_RXSTOP_INIT       (0x1FFF-0x0600-1)
// bufor nadawczy (miejsce na jeden pakiet ethernetowy)
#define ENC28_TXSTART_INIT      (0x1FFF-0x0600)
// koniec bufora nadawczego (koniec pamieci ENC)
#define ENC28_TXSTOP_INIT       0x1FFF

//
// maksymalny rozmiar ramki akceptowalny przez ENC28 (!!!)
//
#define ENC28_MAX_FRAMELEN      1518        // maksymalny rozmiar pakietu w sieci Ethernet - 1518 bajtow

// bufor na pakiety
uint8_t  net_packet[ENC28_MAX_FRAMELEN];

// poczatek kolejnego odebranego pakietu w pamieci ENC28
volatile uint16_t enc28_next_packet_ptr; 

// aktualnie wybrany bank rejestrow
volatile unsigned char enc28_selected_bank;



// inicjalizacja ENC28
void enc28_init();

// inicjalizacja obslugi sieci przez ENC28
void enc28_net_init(unsigned char*, unsigned char*);

// odczyt z / zapis do ENC28
unsigned char enc28_read_opcode(unsigned char, unsigned char);
void enc28_write_opcode(unsigned char, unsigned char, unsigned char);

// wybor aktualnego banku
void enc28_select_bank(unsigned char);

// odczyt / zapis bufora
void enc28_read_buffer(unsigned char*, unsigned int);
void enc28_write_buffer(unsigned char*, unsigned int);

// ustawianie / kasowanie bitow rejestrow (dotyczy tylko rejestrow ETH)
#define enc28_reg_set_bit(address, bits)        enc28_write_opcode(0x80 /* ENC28J60_BIT_FIELD_SET */, address, bits)
#define enc28_reg_clear_bit(address, bits)      enc28_write_opcode(0xA0 /* ENC28J60_BIT_FIELD_CLR */, address, bits)

// odczyt / zapis rejestrow ENC28 (z "automatycznym" wyborem bankow)
unsigned char enc28_read_reg(unsigned char);
void enc28_write_reg(unsigned char, unsigned char);

// odczyt / zapis rejestru typu PHY
uint16_t enc28_read_phy(unsigned char);
void enc28_write_phy(unsigned char, uint16_t);

// wysylka / odbior pakietu
void enc28_packet_send(unsigned char*, unsigned int);
unsigned int enc28_packet_recv(unsigned char*, unsigned int);

// zwraca liczbê pakietów oczekuj¹cych w buforze odbiorczym (rejestr EPKTCNT)
unsigned char enc28_count_packets();

// programowy reset ENC
void enc28_soft_reset();

// odczyt rewizji uk³adu (rev ID)
unsigned char enc28_read_rev_id();

// konfiguracja diod A/B gniazda
#define enc28_setup_led(ledA, ledB)  enc28_write_phy(ENC28_PHLCON, 0x300A | ((ledA) << 8) | ((ledB) << 4) ) // dlugi czas
//#define enc28_setup_led(ledA, ledB)  enc28_write_phy(ENC28_PHLCON, 0x3008 | ((ledA) << 8) | ((ledB) << 4) ) // normalny czas
//#define enc28_setup_led(ledA, ledB)  enc28_write_phy(ENC28_PHLCON, 0x3000 | ((ledA) << 8) | ((ledB) << 4) ) // krotki czas

// czy lacze jest fizyczne podlaczone do ukladu?
unsigned char enc28_is_link_up();

// czy uklad "zapycha" cale pasmo ethernetowe?
unsigned char enc28_is_jabbering();

// wysyla informacje o stanie ENC28 przez RS232
void enc28_dump();

#endif
