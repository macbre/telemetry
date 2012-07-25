#include "daq.h"

unsigned int daq_handle_packet(unsigned char* data) {

    unsigned int len = 0;

    rs_text_P(PSTR("Pakiet DAQ: ")); rs_send(data[0]); rs_send(data[1]); rs_newline();

    // rozdzielaj zadania
    switch(data[0]) {

        // informacje o systemie
        case DAQ_CMD_INFO:

            strcpy_P((char*)data, PROGRAM_NAME); len += strlen_P(PROGRAM_NAME);
            data[len++] = ' ';
            strcpy_P((char*)data+len, PROGRAM_VERSION1); len += strlen_P(PROGRAM_VERSION1);
            data[len++] = ' ';
            strcpy_P((char*)data+len, PROGRAM_VERSION2); len += strlen_P(PROGRAM_VERSION2);

            return len;

            break;

        // liczba czujników temperatury
        case DAQ_CMD_COUNT_SENSORS:

            // little endian - intel / avr
            data[0] = ds_devices_count;
            data[1] = 0;

            return 2;

            break;

        // odczyt danych
        case DAQ_CMD_READ:
            
            switch(data[1]) {
                // odczyt temperatury
                case DAQ_CMD_READ_TEMPERATURE:
                    return daq_read_temperature(data);

                // wype³nienia kana³ów PWM
                case DAQ_CMD_READ_PWM_FILL:
                    for (len=0; len < PWM_CHANNELS; len++) {
                        data[len] = pwm_get_fill(len);
                    }
                    return PWM_CHANNELS * sizeof(unsigned char);
            }

            break;

        // zapis danych
        case DAQ_CMD_SET:
            
            switch(data[1]) {
                // ustawienie wype³nienia kana³u PWM
                case DAQ_CMD_SET_PWM_FILL:
                    pwm_set_fill( (data[2]-'0') % PWM_CHANNELS, atoi( (char*)data+3) );
                    return 0;
            }

            break;
    }

    // zwróæ informacjê o b³êdzie
    data[0] = 'e';
    data[1] = 'r';
    data[2] = 'r';

    return 3;
}



unsigned int daq_read_temperature(unsigned char* data) {

    // tablica na pomiary
    signed int temp[DS_DEVICES_MAX]; /* int16 */
    unsigned int size = sizeof(signed int) * ds_devices_count;
    unsigned char n;

    // przepisz pomiary do tablicy tymczasowej
    for (n=0; n < ds_devices_count; n++) {
        temp[n] = ds_temp[n];
    }

    // skopiuj pomiary do pakietu UDP
    memcpy((void*)data, (void*)temp, size);

    return size;
}

unsigned int daq_start(char* name, unsigned char ch, unsigned int interval, unsigned int samples) {

    // trwa ju¿ inne zadanie akwizycji - poczekaj na jego zakoñczenie
    if (daq_task.samples > 0) {
        return 0;
    }

    // dalsze sprawdzenie poprawnoœci parametrów
    if ( (ch >= ds_devices_count) || (interval < 1) || (interval > 3600) || (samples < 1) || (samples > 200) || !strlen(name) ) {
        return 0;
    }

    // utwórz plik na wyniki pomiarów
    if (!fs_open(&(daq_task.fp), (unsigned char*)name, 0) || daq_task.fp.size > 0) {
        return 0;
    }

    // kopiuj dane do struktury zadania DAQ
    memcpy((void*)daq_task.name, (void*)name, strlen(name));
    daq_task.ch = ch;
    daq_task.interval = interval;
    daq_task.samples = samples;

    //
    // funkcja daq_pooling() dokonuje od teraz okresowego (co <interval> sekund) pomiaru <samples> próbek
    //

    rs_text_P(PSTR("DAQ: rozpoczeto rejestracje do pliku ")); rs_text((char*)(daq_task.fp.name)); rs_send('\''); rs_newline();

    return 1;
}

void daq_pooling() {
    daq_interval++;

    // jeszcze / ju¿ nie teraz
    if ( (daq_interval < daq_task.interval) || (daq_task.samples == 0) ) 
        return;

    // dokonaj odczytu z kana³u <ch>
    signed int readout = ds_temp[daq_task.ch];

    // debug
    rs_send('D'); rs_int(readout); rs_newline();

    // zapisz do pliku
    fs_write(&(daq_task.fp), (unsigned char*)&readout, sizeof(signed int));

    daq_interval = 0;
    daq_task.samples--;

    // zakoñczono zadanie
    if (daq_task.samples == 0) {
        rs_text_P(PSTR("DAQ: zakonczono rejestracje do pliku '")); rs_text((char*)(daq_task.fp.name)); rs_send('\''); rs_newline();
        fs_close(&(daq_task.fp));
    }
}
