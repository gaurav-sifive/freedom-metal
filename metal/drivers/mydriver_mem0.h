#ifndef METAL__DRIVERS__MYDRIVER_MEM0_H
#define METAL__DRIVERS__MYDRIVER_MEM0_H

#include <stdint.h>

#define ADDR *(volatile uint32_t *)(0x80010000)
#define INIT_VAL 0xAA

void my_mem_driver_init();
void my_mem_driver_write(uint32_t *);
void my_mem_driver_read(uint32_t *);

#endif
 