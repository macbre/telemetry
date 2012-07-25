#ifndef _PID_H
#define _PID_H

// @see AVR221 - Discrete PID controller

// wspolczynnik skalowania wsp. P, I, D
#define PID_SCALING_FACTOR  128

// regulator PID
typedef struct
{
  int last_PV;      // ostatnia wartosc procesowa (PV)
  long sum_e;       // calka po uchybie
  
  unsigned int P;   // P: wzmocnienie czesci proporcjonalnej
  unsigned int I;   // I: wzmocnienie czesci calkujacej
  unsigned int D;   // D: wzmocnienie czesci rozniczkujacej

  int max_e;        // maksymalny dopuszczalny uchyb
  long max_sum_e;   // maksymalna dopuszczalna calka po uchybie

} pid_regulator;

#include "../telemetry.h"

// inicjalizacja regulatora PID z podanymi wsp. P, I, D
void pid_init(pid_regulator*, unsigned int, unsigned int, unsigned int);

// cykl pracy regulatora -> aktualizuj wartosci P, I, D wg podanych wartosci SP i PV -> zwroc wysterowanie
int pid_loop(pid_regulator*, int, int);

// reset integrator w strukturze regulatora PID
void pid_reset(pid_regulator*);

// zrzut stanu regulatora do konsoli
void pid_dump(pid_regulator*);

#endif
