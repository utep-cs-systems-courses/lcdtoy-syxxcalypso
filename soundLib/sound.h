#ifndef buzzer_included
#define buzzer_included


void init_buzzer();
void set_buzzer(short cycles);
void stop_buzzer();
void test_buzzer();
void handle_switch_irq();

#endif // included

