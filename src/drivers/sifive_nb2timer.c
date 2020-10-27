/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifdef METAL_SIFIVE_NB2TIMER

#include <metal/drivers/sifive_nb2timer.h>
#include <metal/machine.h>
#include <metal/machine/platform.h>

void timer_set_ctrl(uint32_t timer_id, uint32_t val) {
    if(timer_id < NUM_TIMER_COUNT){
        *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_CONTROL_REG) = val;
    }
}

uint32_t timer_get_ctrl(uint32_t timer_id) {
    if(timer_id < NUM_TIMER_COUNT){
        return *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_CONTROL_REG);
    }
}

uint32_t timer_get_interrupt_status(uint32_t timer_id) {
    if(timer_id < NUM_TIMER_COUNT){
        return *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_INT_STATUS);
    }   
}

void timer_clr_interrupt_status(uint32_t timer_id) {
    if(timer_id < NUM_TIMER_COUNT){
        uint32_t temp_val;
        temp_val = *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_EOI);
    } 
}

void timer_load1(uint32_t timer_id, uint32_t val) {
    if(timer_id < NUM_TIMER_COUNT){
        *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_LOAD_COUNT) = val;
    }

}

void timer_load2(uint32_t timer_id, uint32_t val) {
    if(timer_id < NUM_TIMER_COUNT){
        *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_LOAD_COUNT2) = val;
    }
}

uint32_t timer_get_count(uint32_t timer_id) {
    if(timer_id < NUM_TIMER_COUNT){
        return *(uint32_t *)((METAL_SIFIVE_NB2TIMER_0_BASE_ADDRESS + (timer_id)*0x14) + METAL_SIFIVE_NB2TIMER_TIMER_N_CURRENT_VALUE);
    }
}

#endif
