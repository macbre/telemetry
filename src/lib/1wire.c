#include "1wire.h"


// reset magistrali 1wire (zwraca informacje czy na magistrali dziala slave)
unsigned char ow_reset()
{
    unsigned char presence;

    // linia jako wyjœcie (œci¹gniête do masy)
    OW_ZERO;
    OW_OUTPUT;	

    _delay_loop_2(OW_DELAY_H);     // czekaj...

    // linia jako wejœcie (bez podci¹gania)
    OW_INPUT;

    // czekaj na zg³oszenie (powinno trwaæ przez 60us po 60us od zwolnienia linii przez mastera)
    _delay_loop_2(OW_DELAY_I);


    // sprawdz czy slave sciagnal linie do masy
    if (OW_READ == 0)
        presence = 1;
    else
        presence = 0;

    // czekaj na zwolnienie linii przez slave'a
    _delay_loop_2(OW_DELAY_J);
    _delay_us(20);

    // sprawdz czy sciagniecie do masy nie bylo wynikiem zwarcia
    if (OW_READ == 0)
        presence = 0;
        
    return presence; 		// zwroc informacje czy wykryto slave'a/y na magistrali
} 


// odczyt pojedynczych bitow
unsigned char ow_read_bit()
{
    // master sciaga linie do masy na 5us
	OW_ZERO;
    OW_OUTPUT;

    _delay_us(OW_DELAY_A);

    // zwalniamy linie
    OW_INPUT;

    // czekamy na odpowiedz slave'a
    _delay_us(OW_DELAY_E);

    // pobierz wartosc (0 - slave sciaga linie do masy, 1 - slave zwolnil linie)
    unsigned char bit = OW_READ;

    // czekamy do konca szczeliny czasowej
    _delay_loop_2(OW_DELAY_F);

    return bit ? 1 : 0;
}

// zapis pojedynczych bitow
void ow_write_bit(unsigned char bit)
{
    // master sciaga linie do masy na 5us
	OW_ZERO;
    OW_OUTPUT;

    // jesli zapisujemy "1" zwalniamy linie do konca szczeliny czasowej
    if (bit == 1)
        _delay_us(OW_DELAY_A);
    else
        _delay_loop_2(OW_DELAY_C);
    
    OW_INPUT;
    
    // czekamy do konca szczeliny czasowej
    if (bit == 1)
        _delay_loop_2(OW_DELAY_B);
    else
        _delay_us(OW_DELAY_D);
}



// odczyt bajtow
unsigned char ow_read()
{
    // nie pozwol na przerwanie operacji przez przerwanie
    unsigned char sreg;
    sreg=SREG;
    cli();

    unsigned char tmp = 0x00;
    unsigned char b;

    for (b=0; b<8; b++)
    {
        tmp |= (ow_read_bit() & 0x01) << b;
    }

    // przywroc przerwania
    SREG = sreg;

    return tmp;
}


// zapis bajtow
void ow_write(unsigned char value)
{
    // nie pozwol na przerwanie operacji przez przerwanie
    unsigned char sreg;
    sreg=SREG;
    cli();

    unsigned char tmp, b;

    for(b=0; b<8; b++)
	{	
        tmp = value>>b;             // przesun wysylany bajt
        ow_write_bit(tmp & 0x01);   // wyslij LSB
	}


    // przywroc przerwania
    SREG = sreg;

    _delay_us(200);
}




// odczytaj kod ROM (tylko dla jednego slave'a!)
void ow_read_rom_code(unsigned char* code)
{
    unsigned char b;

    ow_reset();
    ow_write(0x33);

    for (b=0; b<8; b++)
        code[b] = ow_read();

    return;
}

// wybierz (Match ROM) slave'a na magistrali 1wire
void ow_match_rom(unsigned char* code)
{
    unsigned char b;

    ow_reset();
    ow_write(0x55);

    for (b=0; b<8; b++)
        ow_write(code[b]);

    return;
}






// znajdz pierwsze urzadzenie 1wire 
unsigned char ow_first_search()
{   
	LastDiscrepancy = 0;
	LastDeviceFlag  = 0;			// reset the search state
	LastFamilyDiscrepancy = 0;

	return ow_search();
}

// znajdz nastepne urzadzenie 1wire 
unsigned char ow_next_search()
{
    return ow_search();		// leave the search state alone
}


// znajdz urzadzenia 1wire
unsigned char ow_search()
{
   	unsigned char id_bit_number;
   	unsigned char last_zero, rom_byte_number, search_result;
   	unsigned char id_bit, cmp_id_bit;
   	unsigned char rom_byte_mask, search_direction;

   	// initialize for search
   	id_bit_number = 1;
	last_zero = 0;
   	rom_byte_number = 0;
   	rom_byte_mask = 1;
   	search_result = 0;
   	crc8 = 0;

   	// if the last call was not the last one
   	if (!LastDeviceFlag)
   	{      	
      	if ( !ow_reset() ) // 1-Wire reset
      	{         	
         	LastDiscrepancy = 0;// reset the search
         	LastDeviceFlag = 0;
         	LastFamilyDiscrepancy = 0;
         	return 0;
      	}


      	ow_write(0xF0);  // issue the search command 

      
      	do				// loop to do the search
  		{
         	id_bit = ow_read_bit();		// read a bit and its complement
         	cmp_id_bit = ow_read_bit();

         	// check for no devices on 1-wire
         	if ((id_bit == 1) && (cmp_id_bit == 1))
    		{
            	break;
    		}
         	else
         	{
            	// all devices coupled have 0 or 1
            	if (id_bit != cmp_id_bit)
    			{
               		search_direction = id_bit;  // bit write value for search
    			}
            	else
           		{
               		// if this discrepancy if before the Last Discrepancy
               		// on a previous next then pick the same as last time
               		if (id_bit_number < LastDiscrepancy)
    				{
                  		search_direction = ((OW_ROM[rom_byte_number] & rom_byte_mask) > 0);
    				}
               		else
    				{
                  		// if equal to last pick 1, if not then pick 0
                  		search_direction = (id_bit_number == LastDiscrepancy);
    				}
               		// if 0 was picked then record its position in LastZero
               		if (search_direction == 0)
               		{
                  		last_zero = id_bit_number;

                  		// check for Last discrepancy in family
                  		if (last_zero < 9)
    					{
                     		LastFamilyDiscrepancy = last_zero;
    					}
               		}
            	}

        		// set or clear the bit in the ROM byte rom_byte_number
        		// with mask rom_byte_mask
        		if (search_direction == 1)
    			{
          			OW_ROM[rom_byte_number] |= rom_byte_mask;
    			}
        		else
    			{
          			OW_ROM[rom_byte_number] &= ~rom_byte_mask;
    			}

                // serial number search direction write bit
                ow_write_bit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0)
                {
                    //docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
     		}
      	}
  		while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7
      			
        // if the search was successful then
		if (!((id_bit_number < 65) ))//|| (crc8 != 0)))
		{
 			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
 			LastDiscrepancy = last_zero;

 			// check for last device
 			if (LastDiscrepancy == 0)
			{
    		    LastDeviceFlag = 1;
			}
            search_result = 1;
		}
	}
            
    //if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !OW_ROM[0])
   	{
      	LastDiscrepancy = 0;
      	LastDeviceFlag = 0;
      	LastFamilyDiscrepancy = 0;
      	search_result = 0;
   	}

   	return search_result;
}

