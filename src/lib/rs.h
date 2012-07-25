#ifndef _RS_H_
#define	_RS_H_

#include "../telemetry.h"

#include <avr/pgmspace.h>

// okreslenie wartosci wpisu do UBBR
#define USART_BAUDRATE(br) (F_CPU/1000*125/br-1)

// inicjalizacja UART z okreslona pràdkoscia
void rs_init(unsigned long);

// odbiera dane z bufora (procedura blokujaca!)
unsigned char rs_recv();

// wysy³a dane do bufora (procedura blokujaca!)
void rs_send(unsigned char);

// sprawdza czy w buforze znajduje sie nieodczytane odebrane dane (USART Receive Complete)
unsigned char rs_has_recv();

// sprawdza czy dane w buforze zosta³y wys³ane (USART Transmit Complete)
unsigned char rs_has_send();

// wysy³a ³añcuch tekstowy (procedura blokuj¹ca!)
void rs_text(char[]);

// wysy³a ³añcuch tekstowy mieszcz¹cy siê w pamiêci programu
void rs_text_P(PGM_P);

// zaczyna now¹ liniê (ci¹g CRLF)
#define rs_newline()    rs_send(10);rs_send(13);

void rs_int(int);
void rs_long(unsigned long);
void rs_int2(unsigned char);

void rs_hex(unsigned char);

// zrzyut pamieci spod podanego wskaznika
void rs_dump(unsigned char*, unsigned int);

#endif
