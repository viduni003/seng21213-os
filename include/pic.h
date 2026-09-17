#ifndef PIC_H
#define PIC_H
#include "types.h"
void pic_remap(void);
void pic_send_eoi(uint8_t irq);
#endif
void pic_unmask_irq(uint8_t irq);
