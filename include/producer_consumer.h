#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include "types.h"

void pc_init(void);
void producer_thread(void);
void consumer_thread(void);
uint32_t pc_get_produced(void);
uint32_t pc_get_consumed(void);

void race_incrementer_a(void);
void race_incrementer_b(void);
uint32_t race_get_counter(void);
void race_reset_counter(void);

#endif
