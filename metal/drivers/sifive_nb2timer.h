/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__DRIVERS_SIFIVE_NB2TIMER_H
#define METAL__DRIVERS_SIFIVE_NB2TIMER_H

/*!
 * @file sifive_nb2timer.h
 *
 * @brief  APIs to access and control NB2 General Purpose Timers.
 */

#include <stdint.h>

/*! @brief Write 'val' into timer control register.
 * @param timer_id ID of the timer which needs to be accessed.
 * @param val Timer control configuration value.
 * 
 * @return None. */  
void timer_set_ctrl(uint32_t timer_id, uint32_t val);

/*! @brief Returns current timer control register value.
 * @param timer_id ID of the timer which needs to be accessed.
 * @return Current timer control register value. */ 
uint32_t timer_get_ctrl(uint32_t timer_id);

/*! @brief Returns interrupt status for given timer.
 * @param timer_id ID of the timer which needs to be accessed.
 * @return 0x01 Interrupt Active 0x00 Interrupt Inactive. */
uint32_t timer_get_interrupt_status(uint32_t timer_id);

/*! @brief Clears interrupt status for given timer. The register used, TimerNEOI, is Read Only which will return 0 on reading. 
 * @param timer_id ID of the timer which needs to be accessed.
 * @return None */
void timer_clr_interrupt_status(uint32_t timer_id);

/*! @brief Loads value into timer count 1 register.
 * @param timer_id ID of the timer which needs to be accessed.
 * @param val Integer value to be loaded in timer count register.
 * @return None */
void timer_load1(uint32_t timer_id, uint32_t val);

/*! @brief Loads value into timer count 2 register.
 * @param timer_id ID of the timer which needs to be accessed.
 * @param val Integer value to be loaded in timer count register.
 * @return None. */
void timer_load2(uint32_t timer_id, uint32_t val);

/*! @brief Returns current timer count value.
 * @param timer_id ID of the timer which needs to be accessed.
 * @return Current timer count value. */ 
uint32_t timer_get_count(uint32_t timer_id);

#endif