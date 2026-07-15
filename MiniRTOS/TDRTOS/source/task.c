/**
 * @file    task.c
 * @brief   TDRTOS — Khởi tạo Task và Stack Frame cho Cortex-M4
 *
 * ---------------------------------------------------------------
 * CORTEX-M4 EXCEPTION STACK FRAME (khi hardware push tự động):
 *
 *  HIGH ADDRESS (Top of stack khi task bị interrupt)
 *  +----------+
 *  |   xPSR   |  <- Bit 24 (Thumb) phải = 1
 *  +----------+
 *  |    PC    |  <- Địa chỉ hàm task (entry point)
 *  +----------+
 *  |    LR    |  <- Return address (EXC_RETURN hoặc 0xFFFFFFFD)
 *  +----------+
 *  |    R12   |
 *  +----------+
 *  |    R3    |
 *  +----------+
 *  |    R2    |
 *  +----------+
 *  |    R1    |
 *  +----------+
 *  |    R0    |  <- Tham số task (param)
 *  +----------+
 *  |    R11   |  \
 *  +----------+   |
 *  |    R10   |   |
 *  +----------+   |  Software-saved registers
 *  |    R9    |   |  (PendSV handler tự push/pop)
 *  +----------+   |
 *  |    R8    |   |
 *  +----------+   |
 *  |    R7    |   |
 *  +----------+   |
 *  |    R6    |   |
 *  +----------+   |
 *  |    R5    |   |
 *  +----------+   |
 *  |    R4    |  /
 *  +----------+  <- SP trỏ tới đây khi task bị context switch ra
 *
 *  LOW ADDRESS
 * ---------------------------------------------------------------
 */

#include "task.h"
#include <string.h>

/* ============================================================
 *  TDRTOS_TaskCreate
 * ============================================================ */
__attribute__((noreturn)) void TDRTOS_TaskExitError(void) {
  while (1) {
  }
}
int TDRTOS_TaskCreate(TCB_t *tcb, TaskFunction_t taskFunc, void *param,
                      uint32_t *stack, uint32_t stackSize, uint8_t priority,
                      const char *name) {
  /* --- Kiểm tra tham số --- */
  if ((tcb == NULL) || (taskFunc == NULL) || (stack == NULL)) {
    return -1;
  }
  if (stackSize < TDRTOS_MIN_STACK_SIZE) {
    return -1;
  }
  if (priority >= TDRTOS_MAX_PRIORITY) {
    return -1;
  }

  /* --- Fill toàn bộ stack với pattern để phát hiện overflow --- */
  for (uint32_t i = 0; i < stackSize; i++)
  {
    stack[i] = TDRTOS_STACK_FILL_PATTERN;
  }

  /* --- Khởi tạo TCB fields --- */
  tcb->stackBase = stack;
  tcb->stackSize = stackSize;
  tcb->name = name;
  tcb->priority = priority;
  tcb->state = TASK_READY;
  tcb->delayTicks = 0;
  tcb->next = NULL;

  /*
   * --- Xây dựng Initial Stack Frame ---
   *
   * Stack trên Cortex-M4 tăng xuống (full-descending).
   * Bắt đầu từ đỉnh stack (địa chỉ cao nhất),
   * đặt exception frame theo thứ tự hardware push:
   *   [xPSR, PC, LR, R12, R3, R2, R1, R0]
   * Tiếp theo là software-saved regs:
   *   [R11, R10, R9, R8, R7, R6, R5, R4]
   * SP cuối cùng trỏ vào R4 (bottom của full frame).
   */

  /* Con trỏ bắt đầu từ top of stack */
  uint32_t *pStack = stack + stackSize;
  pStack = (uint32_t *)((uint32_t)pStack & ~0x7UL);
  /* -- Hardware exception frame (push từ trên xuống dưới) -- */

  /* Căn chỉnh 8-byte theo ARM ABI (không cần với uint32_t align) */

  *(--pStack) = INITIAL_XPSR;       /* xPSR — Thumb bit set             */
  *(--pStack) = (uint32_t)taskFunc; /* PC   — entry point của task      */
  *(--pStack) = (uint32_t)INITIAL_EXC_RETURN;
   /* LR   — EXC_RETURN                */

  *(--pStack) = 0x0000000CUL;    /* R12                               */
  *(--pStack) = 0x00000003UL;    /* R3                                */
  *(--pStack) = 0x00000002UL;    /* R2                                */
  *(--pStack) = 0x00000001UL;    /* R1                                */
  *(--pStack) = (uint32_t)param; /* R0  — tham số task               */

  /* -- Software-saved registers (PendSV handler save/restore) -- */
  // *(--pStack) = INITIAL_EXC_RETURN;   /* LR (EXC_RETURN cho PendSV)        */
  *(--pStack) = 0x0000000BUL; /* R11                               */
  *(--pStack) = 0x0000000AUL; /* R10                               */
  *(--pStack) = 0x00000009UL; /* R9                                */
  *(--pStack) = 0x00000008UL; /* R8                                */
  *(--pStack) = 0x00000007UL; /* R7                                */
  *(--pStack) = 0x00000006UL; /* R6                                */
  *(--pStack) = 0x00000005UL; /* R5                                */
  *(--pStack) = 0x00000004UL; /* R4                                */

  /* SP trỏ tới vị trí R4 (bottom của initial stack frame) */
  tcb->sp = (volatile uint32_t *)pStack;

  return 0; /* Thành công */
}

/* ============================================================
 *  TDRTOS_TaskCheckStackOverflow
 * ============================================================ */

int TDRTOS_TaskCheckStackOverflow(const TCB_t *tcb)
{
  if (tcb == NULL)
    return 1;

  /*
   * Kiểm tra word đầu tiên của stack (địa chỉ thấp nhất).
   * Nếu pattern bị ghi đè → stack overflow.
   */
  if (tcb->stackBase[0] != TDRTOS_STACK_FILL_PATTERN)
  {
    return 1; /* Overflow! */
  }
  return 0; /* OK */
}

/* ============================================================
 *  TDRTOS_TaskGetState
 * ============================================================ */

TaskState_t TDRTOS_TaskGetState(const TCB_t *tcb) {
  if (tcb == NULL)
    return TASK_DELETED;
  return tcb->state;
}
