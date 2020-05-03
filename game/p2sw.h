#ifndef P2SW_H
#define P2SW_H

#include "msp430.h"

unsigned int p2sw_read();
void p2sw_init(unsigned char mask);
//void HandleSwitchIRQ();

#endif // P2SW_H
