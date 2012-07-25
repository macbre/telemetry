#ifndef _DAQ_H
#define _DAQ_H

#include "../telemetry.h"

#define DAQ_PORT       4192


#define DAQ_CMD_INFO                'i'
#define DAQ_CMD_COUNT_SENSORS       'c'

#define DAQ_CMD_READ                'r'
#define DAQ_CMD_READ_SET_POINTS     's'
#define DAQ_CMD_READ_TEMPERATURE    't'
#define DAQ_CMD_READ_PWM_FILL       'f'
#define DAQ_CMD_READ_PID_OUTPUT     'p'

#define DAQ_CMD_SET                 's'
#define DAQ_CMD_SET_PWM_FILL        'f'

unsigned int daq_handle_packet(unsigned char*);

unsigned int daq_read_temperature(unsigned char*);
//unsigned int daq_read_pwm(unsigned char*);

// start akwizycji
unsigned int daq_start(char*, unsigned char, unsigned int, unsigned int);

// próbuj dokonaæ akwizycji co sekundê
void daq_pooling();

// zadanie akwizycji
struct {
    char name[9];
    unsigned char ch;
    unsigned int interval;
    unsigned int samples;
    fs_file fp;
} daq_task;

// licznik sekund w okresie akwizycji
unsigned int daq_interval;

#endif
