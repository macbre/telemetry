// HD44780
#include "lcd.h"

// ustaw wartoœæ RS
void lcd_rs(unsigned char value)
{
    _delay_us(1);
	    (value == 1) ? sbi(LCD_RS_PORT, LCD_RS_BIT) : cbi(LCD_RS_PORT, LCD_RS_BIT);
    _delay_us(1);
}

// ustaw wartoœæ E
void lcd_e(unsigned char value)
{
    _delay_us(1);
	    (value == 1) ? sbi(LCD_E_PORT, LCD_E_BIT) : cbi(LCD_E_PORT, LCD_E_BIT);
    _delay_us(1);
}

// wysy³a czwórkê bitów (nibble) do LCD (linie D4-D7)
void lcd_nibble(unsigned char nibble)
{
	(nibble & 0x1) ? sbi(LCD_D4_PORT, LCD_D4_BIT) : cbi(LCD_D4_PORT, LCD_D4_BIT);
	(nibble & 0x2) ? sbi(LCD_D5_PORT, LCD_D5_BIT) : cbi(LCD_D5_PORT, LCD_D5_BIT);
	(nibble & 0x4) ? sbi(LCD_D6_PORT, LCD_D6_BIT) : cbi(LCD_D6_PORT, LCD_D6_BIT);
	(nibble & 0x8) ? sbi(LCD_D7_PORT, LCD_D7_BIT) : cbi(LCD_D7_PORT, LCD_D7_BIT);

    _delay_us(5);
}

void lcd_send_instr(unsigned char data) {

	lcd_e(0);
    
    lcd_rs(0);	// wysy³amy instrukcjê

	// starszy nibble 0xF-
	lcd_e(1);

	lcd_nibble( data >> 4 );

	lcd_e(0);
	_delay_us(50);_delay_us(50);

	// m³odszy nibble 0x-F
	lcd_e(1);

	lcd_nibble( data & 0x0f);

	lcd_e(0);
	_delay_us(50);_delay_us(50);

}

void lcd_send_data(unsigned char data) {

    lcd_e(0);

	lcd_rs(1);

	// starszy nibble 0xF-
	lcd_e(1);

	lcd_nibble( data >> 4 );

	lcd_e(0);
	_delay_us(50);_delay_us(50);

	// m³odszy nibble 0x-F
	lcd_e(1);

	lcd_nibble( data & 0x0f);

	lcd_e(0);
	_delay_us(50);_delay_us(50);

}

void lcd_clear()
{
	lcd_send_instr(LCD_CMD_CLEAR);
	_delay_ms(2);
}

void lcd_xy(unsigned char x, unsigned char y)
{
	x &= 0x0F;
	y &= 0x01;
	
	lcd_send_instr( (y*0x40 + x) | (0x80) );
}

void lcd_text(char txt[])
{
    unsigned char c;

	while( (c = *(txt++)) != 0x00 )
		lcd_send_data(c);
}

void lcd_text_P(PGM_P txt)
{
	unsigned char c;
  	while ((c = pgm_read_byte(txt++)))
    	lcd_send_data(c);
}

void lcd_char(unsigned char letter)
{
	lcd_send_data(letter);
}

void lcd_int(unsigned int value)
{
	char buf[8];

    lcd_text( itoa(value, buf, 10) );
}

void lcd_int2(unsigned char val)
{
	lcd_send_data( (val / 10)%10 + '0');
	lcd_send_data( (val % 10)    + '0');
}

void lcd_hex(unsigned char val)
{
    lcd_send_data(dec2hex(val >> 4));
    lcd_send_data(dec2hex(val & 0x0f));
}

void lcd_load_chars(const char *s)
{
      register unsigned char l;
      lcd_send_instr(0x40);		//ustaw adres CGRAM na 0
      for(l=0;l<64;l++)
        lcd_send_data(pgm_read_byte(s++));	//wpisuj kolejne definicje do CGRAM
      lcd_send_instr(0x80);		//kursor na pocz¹tek
}

// polski zestaw znakow
#ifdef LCD_USE_POLISH_CHARS

    prog_char lcd_polish_chars_data[64] = {
    	32 , 32 , 14 ,  1 , 15 , 17 , 15 ,  2,	// ¹
    	 2 ,  4 , 14 , 16 , 16 , 16 , 14 , 32,	// æ
    	32 , 32 , 14 , 17 , 30 , 16 , 14 ,  2,	// ê
    	12 ,  4 ,  6 , 12 ,  4 ,  4 , 14 , 32,	// ³
    	 2 ,  4 , 22 , 25 , 17 , 17 , 17 , 32,	// ñ
    	 2 ,  4 , 14 , 17 , 17 , 17 , 14 , 32,  // ó
    	 2 ,  4 , 14 , 16 , 14 ,  1 , 30 , 32,	// œ
    	 4 , 32 , 31 ,  2 ,  4 ,  8 , 31 , 32	// ¿
	};

    
#endif



void lcd_init() {

    unsigned char i;

	// czekaj na stabilizacjê zasilania (150ms)
	for (i=0; i < 150; i++)
        _delay_ms(1);

    LCD_DDR_SETUP

    lcd_e(0);
	lcd_rs(0);

    // inicjalizacja
    //
    // wyslij 0x3- 0x3- 0x3- 0x2-

	for (i=0; i<3; i++) {
		lcd_e(1);
		lcd_nibble(0x3);
		lcd_e(0);
		_delay_ms(5);
	}

	lcd_e(1);
	lcd_nibble(0x2);
	lcd_e(0);

	_delay_ms(10);

    // ustaw parametry pracy wyswietlacza
    //
    lcd_send_instr(LCD_CMD_FUNC | 0x08);    // magistrala 4-bitowa, 2 linie, matryca 5x7    0x28
	lcd_send_instr(LCD_CMD_OP);	            // wylacz wyswietlacz, kursor i jego mruganie   0x08
    lcd_clear();                            // czysc LCD                                    0x01
    lcd_send_instr(LCD_CMD_MODE | 0x02);	// wlacz tryb bez przesuwania zawartosci ekranu 0x0B
    lcd_send_instr(LCD_CMD_OP | 0x04);	    // wlacz wyswietlacz bez kursora i mrugania     0x0B

    // zaladowac definicje polskich znaków?
    #ifdef LCD_USE_POLISH_CHARS
        lcd_load_chars(lcd_polish_chars_data);
    #endif

	// LCD wolne dla wspoldzielenia przez przerwania
	lcd_busy = 0;
}
