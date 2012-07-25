#include "pid.h"

void pid_init(pid_regulator* pid, unsigned int P, unsigned int I, unsigned int D) {

    // zerowanie
    pid->last_PV = 0;
    pid->sum_e = 0L;

    // wart. wspolczynnikow
    pid->P = P;
    pid->I = I;
    pid->D = D;

    // zabezpiecz przed przepelnieniem
    pid->max_e = INT16_MAX / (pid->P + 1);
    pid->max_sum_e = (INT32_MAX/2) / (pid->I + 1);
}

int pid_loop(pid_regulator* pid,int pv, int sp) {

    int  error, p_term, d_term; // 16
    long i_term, ret, temp;     // 32

    // oblicz uchyb
    error = sp - pv;

    //
    // P z ograniczeniami
    //
    if (error > pid->max_e) {
        p_term = INT16_MAX;
    }
    else if (error <= -pid->max_e) {
        p_term = INT16_MIN;
    }
    else {
        p_term = pid->P * error;
    }

    //
    // I z anti-windup'em
    //
    
    // calkuj uchyb
    temp = pid->sum_e + error;

    if (temp > pid->max_sum_e) {
        i_term = INT32_MAX/2;
        pid->sum_e = pid->max_sum_e;
    }
    else if (temp < -pid->max_sum_e) {
        i_term = -INT32_MAX/2;
        pid->sum_e = -pid->max_sum_e;
    }
    else {
        pid->sum_e = temp;
        i_term = pid->I * pid->sum_e;
    }

    //
    // D
    //
    d_term = pid->D * (pid->last_PV - pv);

    pid->last_PV = pv;

    // oblicz wysterowanie na wyjsciu regulatora (wraz ze skalowaniem)
    ret = (p_term + i_term + d_term) / PID_SCALING_FACTOR;

    // ograniczenia (+ antiwindup)
    if (ret > INT16_MAX) {
        ret = INT16_MAX;
        pid->sum_e -= error;
    }
    else if (ret < INT16_MIN) {
        ret = INT16_MIN;
        pid->sum_e -= error;
    }

    return (int) ret;
}

void pid_reset(pid_regulator* pid) {
    pid->sum_e = 0L;
}


void pid_dump(pid_regulator* pid) {
    rs_newline();
    rs_text_P(PSTR("PID: "));

    rs_send('P'); rs_int(pid->P); rs_send(' ');
    rs_send('I'); rs_int(pid->I); rs_send(' ');
    rs_send('D'); rs_int(pid->D); rs_send(' ');

    rs_send('P');rs_send('V'); rs_int(pid->last_PV); rs_send(' ');
    rs_send('S');rs_send('E'); rs_long(pid->sum_e);  rs_send(' ');

    rs_newline();
}
