/*
 * TDRTOS_Config.h
 *
 *  Created on: Jul 9, 2026
 *      Author: admin
 */

#ifndef INCLUDE_TDRTOS_CONFIG_H_
#define INCLUDE_TDRTOS_CONFIG_H_

/* ============================================================
 *  CẤU HÌNH KERNEL
 * ============================================================ */
#define TASK_RAM_TOP  ((uint32_t *)0x20030000)
/** Số task tối đa hệ thống hỗ trợ (bao gồm Idle task) */
#define TDRTOS_MAX_TASKS        8U

/** Kích thước stack mặc định cho mỗi task (đơn vị: words = 4 bytes) */
#define TDRTOS_DEFAULT_STACK_SIZE  256U

/** Stack size tối thiểu (đơn vị: words) — đủ cho initial frame Cortex-M4 */
#define TDRTOS_MIN_STACK_SIZE   64U

/** Stack pattern để phát hiện stack overflow */
#define TDRTOS_STACK_FILL_PATTERN   0xDEADBEEFUL

/** Số mức ưu tiên (0 = cao nhất, TDRTOS_MAX_PRIORITY-1 = thấp nhất) */
#define TDRTOS_MAX_PRIORITY     8U



/** Tần số SysTick (ticks/giây) — mặc định 1000 Hz = 1ms/tick */
#define TDRTOS_TICK_RATE_HZ     1000U

/** CPU clock (Hz) — STM32F429ZIT, PLL từ HSI 16MHz:
 *  PLLM=8 → VCO_in=2MHz, PLLN=180 → VCO=360MHz, PLLP=2 → SYSCLK=180MHz */
#define TDRTOS_CPU_CLOCK_HZ     180000000UL

/** Số cycles mỗi SysTick = CPU_CLOCK / TICK_RATE */
#define TDRTOS_SYSTICK_LOAD     (TDRTOS_CPU_CLOCK_HZ / TDRTOS_TICK_RATE_HZ)
/* ============================================================
 *  HẰNG SỐ CHO STACK FRAME KHỞI TẠO
 * ============================================================ */

/**
 * xPSR: Thumb state bit (bit 24) = 1.
 * Các bit còn lại = 0 (không có condition flags, không có IT state).
 */
#define INITIAL_XPSR        0x01000000UL

/**
 * LR khi return từ exception về Thread mode, dùng PSP.
 * EXC_RETURN = 0xFFFFFFFD:
 *   - bit 3 = 1: Return to Thread mode
 *   - bit 2 = 1: Return to PSP
 *   - bit 1 = 0: (reserved)
 *   - bit 0 = 1: (must be 1)
 */
#define INITIAL_EXC_RETURN  0xFFFFFFFDUL

/* ============================================================
 *  THANH GHI ĐIỀU KHIỂN (SCB) — Cortex-M4
 * ============================================================ */

/** System Control Block base address */
//#ifndef SCB_BASE
#define SCB_BASE                0xE000ED00UL

/** ICSR — Interrupt Control and State Register */
#define SCB_ICSR                (*(volatile uint32_t *)(SCB_BASE + 0x04UL))

/** Bit set PendSV trong ICSR */
#define SCB_ICSR_PENDSVSET_BIT  (1UL << 28)

/** SHPR3 — System Handler Priority Register 3 */
#define SCB_SHPR3               (*(volatile uint32_t *)(SCB_BASE + 0x18UL))

/** Priority của PendSV (byte 2 trong SHPR3, priority 0xFF = thấp nhất) */
#define PENDSV_PRIORITY_LOWEST  0xFF

/* ============================================================
 *  SysTick REGISTERS — Cortex-M4
 * ============================================================ */
#define SYSTICK_BASE            0xE000E010UL
#define SYSTICK_CSR             (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYSTICK_RVR             (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYSTICK_CVR             (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

/* CSR bits */
#define SYSTICK_CSR_ENABLE      (1UL << 0)   /**< Enable SysTick             */
#define SYSTICK_CSR_TICKINT     (1UL << 1)   /**< Enable SysTick interrupt   */
#define SYSTICK_CSR_CLKSOURCE   (1UL << 2)   /**< 1 = Processor clock        */


#define INTERRUPT_DISABLE() do{__asm volatile("MOV R0, #0x1");__asm volatile("MSR PRIMASK,R0");} while(0)
#define INTERRUPT_ENABLE()  do{__asm volatile ("MOV R0, #0x0");__asm volatile ("MSR PRIMASK,R0");} while(0)
#endif /* INCLUDE_TDRTOS_CONFIG_H_ */
