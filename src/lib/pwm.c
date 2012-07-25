#include "pwm.h"

void pwm_init()
{
    // port sterownika PWM jako wyjscie w stanie niskim
    DDR(PWM_PORT) |= PWM_MASK;
    PWM_PORT &= ~PWM_MASK;

    // zeruj pamiec
    memset((void*)pwm_fill, 0x00, 8);
    memset((void*)pwm_fill_buf, 0x00, 8);

    pwm_step = 0;
}

void pwm_loop()
{
    // pierwszy krok w okresie PWM
    // przenies dane z bufora wysterowan
    if (pwm_step == 0) {
        memcpy((void*)pwm_fill, (void*)pwm_fill_buf, 8);
    }
    else if (pwm_step == 1){
        // ustaw wyjscia PWM, jesli wysterowania > 0
        for (unsigned char channel=0; channel<8; channel++) {
            if (pwm_fill[channel] > 0) {
                PWM_PORT |= (1 << channel);
            }
            else {
                PWM_PORT &= ~(1 << channel);
            }
        }
    }
    else {
        // sprawdz czy nie nalezy wylaczyc jednego z wyjsc
        for (unsigned char channel=0; channel<8; channel++) {
            if (pwm_fill[channel] == pwm_step) {
                PWM_PORT &= ~(1 << channel);
            }
        }
    }

    // kolejny krok
    pwm_step++;
}

void pwm_set_fill(unsigned char channel, unsigned char fill) {
    pwm_fill_buf[channel] = fill;
}
