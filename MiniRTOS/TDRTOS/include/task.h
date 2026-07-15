/**
 * @file    task.h
 * @brief   TDRTOS — Task Control Block và API quản lý task
 *
 * Dựa trên FreeRTOS implementation tutorial:
 * https://www.freertos.org/Documentation/02-Kernel/05-RTOS-implementation-tutorial
 *
 * Target: STM32F429ZIT (ARM Cortex-M4)
 */

#ifndef TDRTOS_INCLUDE_TASK_H_
#define TDRTOS_INCLUDE_TASK_H_

#include <stdint.h>
#include <stddef.h>
#include <TDRTOS_Config.h>

/* ============================================================
 *  TRẠNG THÁI TASK
 * ============================================================ */

typedef enum
{
    TASK_READY      = 0U,   /**< Sẵn sàng chạy              */
    TASK_RUNNING    = 1U,   /**< Đang chạy trên CPU          */
    TASK_BLOCKED    = 2U,   /**< Đang chờ (delay, event...)  */
    TASK_SUSPENDED  = 3U,   /**< Bị tạm dừng thủ công        */
    TASK_DELETED    = 4U,   /**< Đã xóa                      */
} TaskState_t;

/* ============================================================
 *  TASK CONTROL BLOCK (TCB)
 *
 *  Lưu ý: Trường đầu tiên PHẢI là sp (stack pointer) để
 *  context switch assembly có thể truy cập đơn giản với
 *  offset 0 từ con trỏ TCB.
 * ============================================================ */

typedef struct TCB_s
{
    /* --- Phải là field đầu tiên (offset 0) --- */
    volatile uint32_t  *sp;             /**< Stack pointer hiện tại của task    */

    /* --- Thông tin stack --- */
    uint32_t           *stackBase;      /**< Địa chỉ đầu (thấp nhất) của stack  */
    uint32_t            stackSize;      /**< Kích thước stack tính bằng words    */

    /* --- Metadata task --- */
    const char         *name;           /**< Tên task (debug)                   */
    uint8_t             priority;       /**< Mức ưu tiên (0 = cao nhất)         */
    volatile TaskState_t state;         /**< Trạng thái hiện tại                */

    /* --- Delay / blocking --- */
    volatile uint32_t   delayTicks;     /**< Số ticks còn lại khi bị block      */

    /* --- Linked list cho ready/delay queue (mở rộng sau) --- */
    struct TCB_s       *next;

} TCB_t;

/* ============================================================
 *  KIỂU HÀM TASK
 * ============================================================ */

/**
 * Prototype của một task function.
 * Task phải là vòng lặp vô hạn và KHÔNG ĐƯỢC return.
 *
 * Ví dụ:
 *   void myTask(void *param)
 *   {
 *       while (1)
 *       {
 *           // ... công việc ...
 *           TDRTOS_Delay(100);
 *       }
 *   }
 */
typedef void (*TaskFunction_t)(void *param);

/* ============================================================
 *  API TẠO TASK
 * ============================================================ */

/**
 * @brief  Khởi tạo một TCB và stack cho task mới.
 *
 * @param  tcb        Con trỏ tới TCB cần khởi tạo
 * @param  taskFunc   Hàm task (phải là vòng lặp vô hạn)
 * @param  param      Tham số truyền vào task
 * @param  stack      Vùng nhớ stack (do caller cấp phát)
 * @param  stackSize  Kích thước stack tính bằng WORDS (uint32_t)
 * @param  priority   Mức ưu tiên (0 = cao nhất)
 * @param  name       Tên task (có thể NULL)
 *
 * @retval 0  Thành công
 * @retval -1 Lỗi tham số
 */
int TDRTOS_TaskCreate(TCB_t          *tcb,
                      TaskFunction_t  taskFunc,
                      void           *param,
                      uint32_t       *stack,
                      uint32_t        stackSize,
                      uint8_t         priority,
                      const char     *name);

/**
 * @brief  Kiểm tra stack overflow dựa vào fill pattern.
 * @param  tcb  Con trỏ TCB cần kiểm tra
 * @retval 1 nếu overflow, 0 nếu OK
 */
int TDRTOS_TaskCheckStackOverflow(const TCB_t *tcb);

/**
 * @brief  Lấy trạng thái hiện tại của task.
 */
TaskState_t TDRTOS_TaskGetState(const TCB_t *tcb);

#endif /* TDRTOS_INCLUDE_TASK_H_ */
