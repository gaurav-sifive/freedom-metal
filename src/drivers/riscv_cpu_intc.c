/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <metal/cpu.h>
#include <metal/drivers/riscv_cpu_intc.h>
#include <metal/generated/cpu.h>
#include <metal/init.h>
#include <metal/interrupt.h>
#include <metal/interrupt_handlers.h>
#include <metal/shutdown.h>
#include <stdbool.h>
#include <stdint.h>

#define get_index(intc) ((intc).__interrupt_index)

static bool init_done[__METAL_DT_NUM_HARTS] = { false };

/* MIE CSR Manipulation */

static void __metal_interrupt_global_enable(void) {
    uintptr_t m;
    __asm__ volatile("csrrs %0, mstatus, %1"
                     : "=r"(m)
                     : "r"(METAL_MIE_INTERRUPT));
}

static void __metal_interrupt_global_disable(void) {
    uintptr_t m;
    __asm__ volatile("csrrc %0, mstatus, %1"
                     : "=r"(m)
                     : "r"(METAL_MIE_INTERRUPT));
}

static void __metal_interrupt_software_enable(void) {
    uintptr_t m;
    __asm__ volatile("csrrs %0, mie, %1"
                     : "=r"(m)
                     : "r"(METAL_LOCAL_INTERRUPT_SW));
}

static void __metal_interrupt_software_disable(void) {
    uintptr_t m;
    __asm__ volatile("csrrc %0, mie, %1"
                     : "=r"(m)
                     : "r"(METAL_LOCAL_INTERRUPT_SW));
}

static void __metal_interrupt_timer_enable(void) {
    uintptr_t m;
    __asm__ volatile("csrrs %0, mie, %1"
                     : "=r"(m)
                     : "r"(METAL_LOCAL_INTERRUPT_TMR));
}

static void __metal_interrupt_timer_disable(void) {
    uintptr_t m;
    __asm__ volatile("csrrc %0, mie, %1"
                     : "=r"(m)
                     : "r"(METAL_LOCAL_INTERRUPT_TMR));
}

static void __metal_interrupt_external_enable(void) {
    uintptr_t m;
    __asm__ volatile("csrrs %0, mie, %1"
                     : "=r"(m)
                     : "r"(METAL_LOCAL_INTERRUPT_EXT));
}

static void __metal_interrupt_external_disable(void) {
    unsigned long m;
    __asm__ volatile("csrrc %0, mie, %1"
                     : "=r"(m)
                     : "r"(METAL_LOCAL_INTERRUPT_EXT));
}

static void __metal_interrupt_local_enable(int id) {
    uintptr_t b = 1 << id;
    uintptr_t m;
    __asm__ volatile("csrrs %0, mie, %1" : "=r"(m) : "r"(b));
}

static void __metal_interrupt_local_disable(int id) {
    uintptr_t b = 1 << id;
    uintptr_t m;
    __asm__ volatile("csrrc %0, mie, %1" : "=r"(m) : "r"(b));
}

/* Default handlers */

void __metal_exception_handler(void) __attribute__((interrupt, aligned(128)));
void __metal_exception_handler(void) {
    uint32_t hartid = metal_cpu_get_current_hartid();

    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));

    int id = RISCV_MCAUSE_ID(mcause);

    if (RISCV_MCAUSE_IS_INTERRUPT(mcause)) {
        __metal_local_interrupt_table[id]();
    } else {
        __metal_exception_table[id](metal_cpu_get(hartid), id);
    }
}

/* Interrupt API */

extern void early_trap_vector(void);

void __metal_driver_riscv_cpu_intc_init(
    struct metal_interrupt intc) {

    if (!init_done[get_index(intc)]) {
        /*
         * Set the real trap handler if the value of mtvec is equal to
         * early_trap_vector. If mtvec is not equal to early_trap_vector,
         * that means user has own trap handler, then we don't overwrite it.
         */
        uintptr_t mtvec;
        __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));
        if (mtvec == (uintptr_t)&early_trap_vector) {
#ifdef METAL_HLIC_VECTORED
            __metal_driver_riscv_cpu_intc_set_vector_mode(intc, METAL_VECTORED_MODE);
#else
            __metal_driver_riscv_cpu_intc_set_vector_mode(intc, METAL_DIRECT_MODE);
#endif
        }

        init_done[get_index(intc)] = 1;
    }
}

METAL_CONSTRUCTOR(riscv_cpu_intc_init) {
    for (int i = 0; i < __METAL_DT_NUM_HARTS; i++) {
        struct metal_interrupt intc = (struct metal_interrupt) { i };
        __metal_driver_riscv_cpu_intc_init(intc);
    }
}

extern void __metal_vector_table(void);

int __metal_driver_riscv_cpu_intc_set_vector_mode(
    struct metal_interrupt controller, metal_vector_mode mode) {

    uintptr_t mtvec;
    __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));
    
    /* Zero the mtvec CLIC config bits */
    mtvec &= ~(METAL_MTVEC_CLIC_VECTORED | METAL_MTVEC_CLIC_RESERVED);

    switch (mode) {
    default:
    case METAL_DIRECT_MODE:
        /* Write the trap vector address with the vectored bit unset */
        __asm__ volatile(
            "csrw mtvec, %0" ::"r"((uintptr_t)__metal_exception_handler & ~METAL_MTVEC_CLIC_VECTORED));
        break;
    case METAL_VECTORED_MODE:
        /* Write the jump table address with the vectored bit set */
        __asm__ volatile(
            "csrw mtvec, %0" :: "r"((uintptr_t)__metal_vector_table | METAL_MTVEC_VECTORED));
    }

    return 0;
}

metal_vector_mode __metal_driver_riscv_cpu_intc_get_vector_mode(
    struct metal_interrupt controller) {

    uintptr_t mtvec;
    __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));

    switch (mtvec & METAL_MTVEC_MODE) {
        default:
        case METAL_MTVEC_DIRECT:
            return METAL_DIRECT_MODE;
        case METAL_MTVEC_VECTORED:
            return METAL_VECTORED_MODE;
        case METAL_MTVEC_CLIC:
            return METAL_CLIC_DIRECT_MODE;
        case METAL_MTVEC_CLIC_VECTORED:
            return METAL_CLIC_VECTORED_MODE;
    }
}

int __metal_driver_riscv_cpu_intc_set_privilege(struct metal_interrupt controller,
                                                metal_intr_priv_mode privilege) {
    if (privilege == METAL_INTR_PRIV_M_MODE) {
        return 0;
    }
    return -1;
}

metal_intr_priv_mode
__metal_driver_riscv_cpu_intc_get_privilege(struct metal_interrupt controller) {
    return METAL_INTR_PRIV_M_MODE;
}

int __metal_driver_riscv_cpu_intc_clear(struct metal_interrupt controller,
                                        int id) {
    return -1;
}

int __metal_driver_riscv_cpu_intc_set(struct metal_interrupt controller, int id) {
    return -1;
}

int __metal_driver_riscv_cpu_intc_enable(
    struct metal_interrupt controller, int id) {
    switch (id) {
    case METAL_INTERRUPT_ID_BASE:
        __metal_interrupt_global_enable();
        break;
    case METAL_INTERRUPT_ID_SW:
        __metal_interrupt_software_enable();
        break;
    case METAL_INTERRUPT_ID_TMR:
        __metal_interrupt_timer_enable();
        break;
    case METAL_INTERRUPT_ID_EXT:
        __metal_interrupt_external_enable();
        break;
    case METAL_INTERRUPT_ID_LC0:
    case METAL_INTERRUPT_ID_LC1:
    case METAL_INTERRUPT_ID_LC2:
    case METAL_INTERRUPT_ID_LC3:
    case METAL_INTERRUPT_ID_LC4:
    case METAL_INTERRUPT_ID_LC5:
    case METAL_INTERRUPT_ID_LC6:
    case METAL_INTERRUPT_ID_LC7:
    case METAL_INTERRUPT_ID_LC8:
    case METAL_INTERRUPT_ID_LC9:
    case METAL_INTERRUPT_ID_LC10:
    case METAL_INTERRUPT_ID_LC11:
    case METAL_INTERRUPT_ID_LC12:
    case METAL_INTERRUPT_ID_LC13:
    case METAL_INTERRUPT_ID_LC14:
    case METAL_INTERRUPT_ID_LC15:
        __metal_interrupt_local_enable(id);
        break;
    default:
        return -1;
    }
    return 0;
}

int __metal_driver_riscv_cpu_intc_interrupt_disable(
    struct metal_interrupt controller, int id) {
    switch (id) {
    case METAL_INTERRUPT_ID_BASE:
        __metal_interrupt_global_disable();
        break;
    case METAL_INTERRUPT_ID_SW:
        __metal_interrupt_software_disable();
        break;
    case METAL_INTERRUPT_ID_TMR:
        __metal_interrupt_timer_disable();
        break;
    case METAL_INTERRUPT_ID_EXT:
        __metal_interrupt_external_disable();
        break;
    case METAL_INTERRUPT_ID_LC0:
    case METAL_INTERRUPT_ID_LC1:
    case METAL_INTERRUPT_ID_LC2:
    case METAL_INTERRUPT_ID_LC3:
    case METAL_INTERRUPT_ID_LC4:
    case METAL_INTERRUPT_ID_LC5:
    case METAL_INTERRUPT_ID_LC6:
    case METAL_INTERRUPT_ID_LC7:
    case METAL_INTERRUPT_ID_LC8:
    case METAL_INTERRUPT_ID_LC9:
    case METAL_INTERRUPT_ID_LC10:
    case METAL_INTERRUPT_ID_LC11:
    case METAL_INTERRUPT_ID_LC12:
    case METAL_INTERRUPT_ID_LC13:
    case METAL_INTERRUPT_ID_LC14:
    case METAL_INTERRUPT_ID_LC15:
        __metal_interrupt_local_disable(id);
        break;
    default:
        return -1;
    }
    return 0;
}

int __metal_driver_riscv_cpu_intc_set_threshold(struct metal_interrupt controller,
                                                unsigned int level) {
    return -1;
}

unsigned int
__metal_driver_riscv_cpu_intc_get_threshold(struct metal_interrupt controller) {
    return 0;
}

int __metal_driver_riscv_cpu_intc_set_priority(struct metal_interrupt controller,
                                               int id, unsigned int priority) {
    return -1;
}

unsigned int
__metal_driver_riscv_cpu_intc_get_priority(struct metal_interrupt controller, int id) {
    return 0;
}

int
__metal_driver_riscv_cpu_intc_set_preemptive_level(struct metal_interrupt controller, int id,
                                                   unsigned int level) {
    return -1;
}

unsigned int
__metal_driver_riscv_cpu_intc_get_preemptive_level(struct metal_interrupt controller, int id) {
    return 0;
}

int __metal_driver_riscv_cpu_intc_vector_enable(
    struct metal_interrupt intc, int id, metal_vector_mode mode) {

#ifdef METAL_HLIC_VECTORED
    return 0;
#else
    return -1;
#endif
}

int
__metal_driver_riscv_cpu_intc_vector_disable(
    struct metal_interrupt controller, int id) {
    
#ifdef METAL_HLIC_VECTORED
    return -1;
#else
    return 0;
#endif
}

metal_affinity
__metal_driver_riscv_cpu_intc_affinity_enable(struct metal_interrupt controller,
                                              metal_affinity bitmask, int id) {
    /* Return a 1 for each hart in the bitmask, representing failure. */
    return (metal_affinity) { (1 << __METAL_DT_NUM_HARTS) - 1 };
}

metal_affinity
__metal_driver_riscv_cpu_intc_affinity_disable(struct metal_interrupt controller,
                                               metal_affinity bitmask, int id) {
    return (metal_affinity) { (1 << __METAL_DT_NUM_HARTS) - 1 };
}

metal_affinity
__metal_driver_riscv_cpu_intc_affinity_set_threshold(struct metal_interrupt controller,
                                                     metal_affinity bitmask,
                                                     unsigned int level) {
    return (metal_affinity) { (1 << __METAL_DT_NUM_HARTS) - 1 };
}

unsigned int
__metal_driver_riscv_cpu_intc_affinity_get_threshold(struct metal_interrupt controller,
                                                     int context_id) {
    return 0;
}
