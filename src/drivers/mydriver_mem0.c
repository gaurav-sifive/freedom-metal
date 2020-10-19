#ifdef METAL_MYDRIVER_MEM0

#include <metal/drivers/mydriver_mem0.h>

void my_mem_driver_init(void){
    ADDR = INIT_VAL;
    printf("Initial value at given mem address is: %x\n", ADDR);
}

void my_mem_driver_write(uint32_t *data_in){
   ADDR = *data_in;    
   printf("Data Written Successfully! %x\n", ADDR);
}

void my_mem_driver_read(uint32_t *data_out){
    *data_out = ADDR;
    printf("Data at given mem address is: %x\n", ADDR);
}

#endif