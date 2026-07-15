/*
 * app.c
 *
 *  Created on: Jul 11, 2026
 *      Author: admin
 */

#include "app.h"
/* ====================================================
 * KHAI B�?O TASK CHO DEMO TDRTOS
 * ==================================================== */



/* Bộ đếm đơn giản để quan sát context switch qua debugger */
volatile uint32_t task1Counter = 0;
volatile uint32_t task2Counter = 0;
volatile uint32_t task3Counter = 0;
/* ====================================================
 * TASK FUNCTIONS DEMO
 *
 * Mỗi task là vòng lặp vô hạn. Context switch xảy ra
 * qua SysTick (1ms) và PendSV (assembly).
 *
 * �?ể quan sát: Breakpoint + watch taskXCounter trong debugger.
 * ==================================================== */

/**
 * Task 1 — Priority 0 (cao nhất trong demo)
 * Chạy mỗi 500ms
 */
void Task1Function(void *param)
{
    (void)param;
    while (1)
    {
        task1Counter++;
        /* Bật LED PG13 (LD3 trên Nucleo-F429ZI) nếu có */
        GPIOG->ODR ^= (1U << 13U); /* Toggle PG13 */
        TDRTOS_Delay(1000); /* Delay 500 ticks = 500ms */
    }
}

/**
 * Task 2 — Priority 1
 * Chạy mỗi 250ms
 */
void Task2Function(void *param)
{
    (void)param;
    while (1)
    {
        task2Counter++;
        GPIOG->ODR ^= (1U << 14U); /* Toggle PG14 (LD4) */
        TDRTOS_Delay(500);
    }
}

/**
 * Task 3 — Priority 2 (thấp nhất trong user tasks)
 * Chạy mỗi 100ms
 */
void Task3Function(void *param)
{
    (void)param;
    while (1)
    {
        task3Counter++;
        TDRTOS_Delay(500);
    }
}
