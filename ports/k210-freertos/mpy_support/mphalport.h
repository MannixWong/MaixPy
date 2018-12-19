
#ifndef INCLUDED_MPHALPORT_H
#define INCLUDED_MPHALPORT_H

#include "py/ringbuf.h"
#include "lib/utils/interrupt_char.h"

uint32_t mp_hal_ticks_us(void);
uint32_t mp_hal_ticks_ms(void);
uint32_t mp_hal_ticks_cpu(void);

void mp_hal_delay_us(uint32_t);
void mp_hal_delay_ms(mp_uint_t ms);

#endif
