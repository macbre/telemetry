#include "rs.h"

// inicjalizacja UART z okreskona predkoscia
void rs_init(unsigned long rate)
{
	// Baud rate selection
	UBRRH = (unsigned char) (USART_BAUDRATE(rate) >> 8);
	UBRRL = (unsigned char) USART_BAUDRATE(rate);

	// USART setup (p. 151)
	UCSRA = (1<<U2X);                 // wlacz podwojna predkosc U2x
	UCSRB = (1<<RXEN) | (1<<TXEN);   // wlacz nadajnik i odbiornik
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);  // transmisja asynchroniczna, 8N1

	return; 
}

// odbiera dane z bufora (procedura blokujaca!)
unsigned char rs_recv()
{
	// czekaj na dane
	while ( !(UCSRA & (1<<RXC)) );

	// zwroc zawartosc bufora
	return UDR;
}

// wysyla dane do bufora (procedura blokujaca!)
void rs_send(unsigned char data)
{
	// czekaj na zwolnienie bufora nadawczego
	while ( !( UCSRA & (1<<UDRE)) );

	// wyslij dane
	UDR = data;
}

// sprawdza czy w buforze znajduja sie nieodczytane odebrane dane (USART Receive Complete)
unsigned char rs_has_recv()
{
	return (UCSRA & (1<<RXC));
}

// sprawdza czy dane w buforze zostaly wyslane (USART Transmit Complete)
unsigned char rs_has_send()
{
	return (UCSRA & (1<<UDRE));
}

// wysyla lancuch tekstowy (procedura blokujaca!)
void rs_text(char txt[])
{
	unsigned char c;

	while( (c = *(txt++)) != 0x00 )
		rs_send(c);
}

// wysyla lancuch tekstowy z pamieci programu
void rs_text_P(PGM_P txt)
{
	unsigned char c;
  	while ((c = pgm_read_byte(txt++)))
    	rs_send(c);
}

void rs_int(int value)
{
	char buf[8];

    rs_text( itoa(value, buf, 10) );
}

void rs_long(unsigned long value)
{
	char buf[8];

    rs_text( utoa(value, buf, 10) );
}

void rs_int2(unsigned char value)
{
	rs_send( (value / 10) + 48);
	rs_send( (value % 10) + 48);
}

void rs_hex(unsigned char val)
{
    rs_send(dec2hex(val >> 4));
    rs_send(dec2hex(val & 0x0f));
}

void rs_dump(unsigned char* data, unsigned int len)
{
    unsigned int i, j;

    rs_newline();

    for (i=0; i < len; i++) {
        rs_hex(data[i]);

        if ( (i % 16) == 15) {
            rs_send('|');
            for (j=i-15;j<=i; j++) {
                rs_send( data[j] > 0x10 ? data[j] : '.');
            }
            //rs_newline();
        }
        else {
            rs_send(' ');
        }
    }

    rs_newline();
}
