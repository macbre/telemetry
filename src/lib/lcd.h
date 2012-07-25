#ifndef _LCD_H_
#define	_LCD_H_

#include "../telemetry.h"

// ustawienia

// RS
#define LCD_RS_PORT PORTC
#define LCD_RS_BIT  5

// E
#define LCD_E_PORT PORTC
#define LCD_E_BIT  4

// dane (D4-D7)
#define LCD_D4_PORT PORTC
#define LCD_D4_BIT  3
#define LCD_D5_PORT PORTC
#define LCD_D5_BIT  2
#define LCD_D6_PORT PORTC
#define LCD_D6_BIT  1
#define LCD_D7_PORT PORTC
#define LCD_D7_BIT  0

// maska komend
#define LCD_CMD_CLEAR   0x01
#define LCD_CMD_MOVE    0x02
#define LCD_CMD_MODE    0x04
#define LCD_CMD_OP      0x08
#define LCD_CMD_FUNC    0x20

// \ustawienia

// obsluga wspoldzielenia wyswietlacza LCD przez przerwania
//
// LCD_WAIT
//		lcd_xy...
// 		lcd_text...
// LCD_FREE
//
volatile unsigned char lcd_busy;

//#define LCD_WAIT	while(lcd_busy); lcd_busy = 1;
//#define LCD_FREE	lcd_busy = 0;

#define LCD_WAIT	;
#define LCD_FREE	;


#define LCD_DDR_SETUP   sbi(DDR(LCD_RS_PORT), LCD_RS_BIT); \
                        sbi(DDR(LCD_E_PORT),  LCD_E_BIT);  \
                        sbi(DDR(LCD_D4_PORT), LCD_D4_BIT); \
                        sbi(DDR(LCD_D5_PORT), LCD_D5_BIT); \
                        sbi(DDR(LCD_D6_PORT), LCD_D6_BIT); \
                        sbi(DDR(LCD_D7_PORT), LCD_D7_BIT);


void lcd_rs(unsigned char);
void lcd_e(unsigned char);
void lcd_nibble(unsigned char);
void lcd_send_instr(unsigned char);
void lcd_send_data(unsigned char);
void lcd_clear();
void lcd_xy(unsigned char, unsigned char);
void lcd_text(char[]);
void lcd_text_P(PGM_P);
void lcd_char(unsigned char);
void lcd_int(unsigned int);
void lcd_int2(unsigned char);
void lcd_hex(unsigned char);
void lcd_init();


// definicja polskich znaków
#ifdef LCD_USE_POLISH_CHARS

    #define lcd_pl_a		0x08 
    #define lcd_pl_c		0x09
    #define lcd_pl_e		0x0a
    #define lcd_pl_l		0x0b
    #define lcd_pl_n		0x0c
    #define lcd_pl_o		0x0d
    #define lcd_pl_s		0x0e
    #define lcd_pl_z		0x0f

    #define lcd_pl_test     "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x0"

    void lcd_load_chars(const char *);

#endif // \LCD_USE_POLISH_CHARS

//
// definicje strzalek i innych znakow specjalnych
//

// russian CGROM
/*
#define lcd_larrow  0xdb
#define lcd_rarrow  0xdc
#define lcd_uarrow  0xd9
#define lcd_darrow  0xda

#define lcd_enter   0x7e
*/

// standard CGROM
#define lcd_larrow  0x7f
#define lcd_rarrow  0x7e
#define lcd_block   0xff

#endif
