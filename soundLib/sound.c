#include <msp430.h>
#include "sound.h"
#include <libTimer.h>


void init_buzzer(){                    // Sourced from demo
   /*
       Direct timer A output "TA0.1" to P2.6.
        According to table 21 from data sheet:
          P2SEL2.6, P2SEL2.7, anmd P2SEL.7 must be zero
          P2SEL.6 must be 1
        Also: P2.6 direction must be output
    */
    timerAUpmode();     /* used to drive speaker */
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7;
    P2SEL |= BIT6;
    P2DIR = BIT6;       /* enable output to speaker (P2.6) */
}

/* buzzer clock = 2MHz.  (period of 1k results in 2kHz tone) */
void set_buzzer(short cycles) {
  CCR0 = cycles;
  CCR1 = cycles >> 1;       /* one half cycle */
}

void stop_buzzer(){
    set_buzzer(0);
}

void test_buzzer(){
    set_buzzer(440);
}

