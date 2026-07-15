/**
 * @file    scheduler.h
 * @brief   TDRTOS — Scheduler, SysTick, Context Switch API
 *
 * Target: STM32F429ZIT (ARM Cortex-M4)
 *
 * Context switch sử dụng PendSV exception với priority thấp nhất.
 * SysTick cung cấp time base và trigger PendSV.
 */

#ifndef TDRTOS_INCLUDE_SCHEDULER_H_
#define TDRTOS_INCLUDE_SCHEDULER_H_

#include "task.h"
#include "TDRTOS_Config.h"
/* ============================================================
 *  CẤU HÌNH SCHEDULER
 * ============================================================ */


/* ============================================================
 *  BIẾN GLOBAL ĐƯỢC EXPORT (dùng trong context switch asm)
 * ============================================================ */

/**
 * Con trỏ tới TCB đang chạy.
 * Assembly code đọc/ghi field sp (offset 0) của TCB này.
 * Phải là biến global để PendSV handler (assembly) truy cập.
 */
extern TCB_t * volatile g_currentTCB;

/* ============================================================
 *  API KHỞI TẠO VÀ ĐIỀU KHIỂN SCHEDULER
 * ============================================================ */

/**
 * @brief  Khởi tạo scheduler.
 *         Phải gọi trước TDRTOS_AddTask() và TDRTOS_Start().
 */
void TDRTOS_SchedulerInit(void);

/**
 * @brief  Thêm một task vào scheduler.
 *
 * @param  tcb  Con trỏ tới TCB đã được khởi tạo bởi TDRTOS_TaskCreate().
 * @retval 0  Thành công
 * @retval -1 Lỗi (đầy bảng task, TCB null...)
 */
int TDRTOS_AddTask(TCB_t *tcb);

/**
 * @brief  Bắt đầu chạy scheduler.
 *         Hàm này KHÔNG BAO GIỜ RETURN nếu có ít nhất 1 task.
 *         Nó khởi động SysTick, chọn task đầu tiên và nhảy vào task đó.
 */
void TDRTOS_Start(void);

/**
 * @brief  Delay task hiện tại trong một số ticks.
 *         Task sẽ chuyển sang trạng thái BLOCKED và yield CPU.
 *
 * @param  ticks  Số SysTick ticks cần chờ
 */
void TDRTOS_Delay(uint32_t ticks);

/**
 * @brief  Lấy giá trị tick counter hiện tại.
 * @retval Số ticks kể từ khi scheduler khởi động
 */
uint32_t TDRTOS_GetTickCount(void);

/**
 * @brief  Lấy TCB của task đang chạy.
 * @retval Con trỏ tới TCB hiện tại (NULL nếu scheduler chưa chạy)
 */
TCB_t *TDRTOS_GetCurrentTCB(void);

/* ============================================================
 *  INTERNAL — Gọi từ SysTick Handler
 * ============================================================ */

/**
 * @brief  Xử lý tick: cập nhật delay counter, chọn task tiếp theo,
 *         trigger PendSV để thực hiện context switch.
 *         Gọi từ SysTick_Handler().
 */
void TDRTOS_TickHandler(void);

/**
 * @brief  Chọn task tiếp theo theo thuật toán Round-Robin.
 *         Gọi từ TDRTOS_TickHandler() và PendSV (nếu cần).
 */
void TDRTOS_SelectNextTask(void);
void OS_SysTick_Handler(void);
void OS_PendSV_Handler(void);
#endif /* TDRTOS_INCLUDE_SCHEDULER_H_ */
