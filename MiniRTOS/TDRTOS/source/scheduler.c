/**
 * @file    scheduler.c
 * @brief   TDRTOS — Scheduler, SysTick, Context Switch (PendSV)
 *
 * Target: STM32F429ZIT (ARM Cortex-M4)
 *
 * ---------------------------------------------------------------
 * CÁCH CONTEXT SWITCH HOẠT ĐỘNG:
 *
 * 1. SysTick_Handler() gọi TDRTOS_TickHandler() mỗi 1ms
 * 2. TickHandler cập nhật delay counters, chọn task tiếp theo
 *    và set bit PENDSVSET để trigger PendSV exception
 * 3. PendSV_Handler() (viết bằng assembly) thực hiện:
 *    a. SAVE context của task hiện tại:
 *       - Hardware đã tự push {R0-R3, R12, LR, PC, xPSR} vào PSP stack
 *       - Software push thêm {R4-R11} vào PSP stack
 *       - Lưu PSP vào g_currentTCB->sp
 *    b. SWITCH task: g_currentTCB = task tiếp theo (đã chọn)
 *    c. RESTORE context của task mới:
 *       - Load PSP từ g_currentTCB->sp
 *       - Pop {R4-R11} từ PSP stack
 *       - Return từ exception (hardware tự pop {R0-R3,R12,LR,PC,xPSR})
 * ---------------------------------------------------------------
 */

#include "scheduler.h"
#include <string.h>

/* ============================================================
 *  BIẾN GLOBAL (được truy cập từ assembly)
 * ============================================================ */

/** TCB đang chạy hiện tại — assembly truy cập offset 0 (sp field) */
TCB_t *volatile g_currentTCB = NULL;

/** TCB của task tiếp theo được chọn bởi scheduler */
static TCB_t *volatile s_nextTCB = NULL;

/** Mảng chứa con trỏ tới các TCB đã đăng ký */
static TCB_t *s_taskList[TDRTOS_MAX_TASKS];

/** Số lượng task hiện đang quản lý */
static volatile uint32_t s_taskCount = 0;

/** Index của task đang chạy (cho round-robin) */
static volatile uint32_t s_currentTaskIndex = 0;

/** Tick counter toàn cục */
static volatile uint32_t s_tickCount = 0;

/** Flag: scheduler đã khởi động chưa */
static volatile uint8_t s_schedulerRunning = 0;

/* ============================================================
 *  IDLE TASK
 *  Luôn là task cuối cùng, chạy khi không có task nào READY.
 * ============================================================ */

static uint32_t s_idleTaskStack[TDRTOS_MIN_STACK_SIZE];
static TCB_t s_idleTaskTCB;

static void idleTaskFunction(void *param)
{
  (void)param;
  while (1)
  {
    /* WFI: Wait For Interrupt — tiết kiệm điện khi không có gì làm */
    __asm volatile("WFI");
  }
}

/* ============================================================
 *  MACRO TIỆN ÍCH
 * ============================================================ */

/** Disable interrupt toàn cục (critical section) */
#define TDRTOS_ENTER_CRITICAL() __asm volatile("CPSID I" ::: "memory")

/** Enable interrupt toàn cục */
#define TDRTOS_EXIT_CRITICAL() __asm volatile("CPSIE I" ::: "memory")

/** Data memory barrier */
#define TDRTOS_DMB() __asm volatile("DMB" ::: "memory")

/** Trigger PendSV để thực hiện context switch */
static inline void triggerContextSwitch(void)
{
  /* Set PENDSVSET bit trong ICSR */
  SCB_ICSR = SCB_ICSR_PENDSVSET_BIT;
  TDRTOS_DMB();
}

/* ============================================================
 *  TDRTOS_SchedulerInit
 * ============================================================ */

void TDRTOS_SchedulerInit(void)
{
  /* Xóa danh sách task */
  for (uint32_t i = 0; i < TDRTOS_MAX_TASKS; i++)
  {
    s_taskList[i] = NULL;
  }
  s_taskCount = 0;
  s_currentTaskIndex = 0;
  s_tickCount = 0;
  s_schedulerRunning = 0;
  g_currentTCB = NULL;
  s_nextTCB = NULL;

  /*
   * Cấu hình PendSV với priority thấp nhất (0xFF).
   * SHPR3 byte[2] = PendSV priority.
   * Điều này đảm bảo context switch chỉ xảy ra sau khi
   * tất cả ISR khác hoàn thành.
   */
  SCB_SHPR3 |= ((uint32_t)PENDSV_PRIORITY_LOWEST << 16U);

  /* Tạo Idle Task (priority thấp nhất) */
  TDRTOS_TaskCreate(&s_idleTaskTCB, idleTaskFunction, NULL, s_idleTaskStack,
                    TDRTOS_MIN_STACK_SIZE,
                    TDRTOS_MAX_PRIORITY - 1U, /* Thấp nhất */
                    "IDLE");
  /* Thêm idle task vào danh sách nội bộ (không qua TDRTOS_AddTask) */
  s_taskList[0] = &s_idleTaskTCB;
  s_taskCount = 1;
}

/* ============================================================
 *  TDRTOS_AddTask
 * ============================================================ */

int TDRTOS_AddTask(TCB_t *tcb)
{
  if (tcb == NULL)
    return -1;

  TDRTOS_ENTER_CRITICAL();

  if (s_taskCount >= TDRTOS_MAX_TASKS)
  {
    TDRTOS_EXIT_CRITICAL();
    return -1;
  }

  /*
   * Chèn task mới vào vị trí trước idle task (index 0).
   * Idle task luôn ở index 0, các task người dùng từ index 1 trở đi.
   * Đơn giản hơn: ta thêm vào cuối, idle task ở đầu không bị ảnh hưởng.
   */
  s_taskList[s_taskCount] = tcb;
  s_taskCount++;

  TDRTOS_EXIT_CRITICAL();
  return 0;
}

/* ============================================================
 *  TDRTOS_SelectNextTask — Round-Robin Scheduler
 * ============================================================ */

void TDRTOS_SelectNextTask(void)
{
  uint32_t startIndex = s_currentTaskIndex;
  uint32_t nextIndex;
  uint8_t highestPriority = TDRTOS_MAX_PRIORITY;
  /* Sẽ so sánh nhỏ hơn */ int32_t selectedIndex = -1;
  /* * Thuật toán: ưu tiên cao nhất (số nhỏ nhất).
   * * Trong các task cùng ưu tiên: Round-Robin. *
   * Chỉ xét các task ở trạng thái READY hoặc RUNNING. */ /* Tìm task READY với priority cao nhất */
  for (uint32_t i = 0; i < s_taskCount; i++)
  { /* Duyệt vòng tròn bắt đầu từ task KẾ tiếp (round-robin) */
    nextIndex = (startIndex + 1 + i) % s_taskCount;
    TCB_t *candidate = s_taskList[nextIndex];
    if (candidate == NULL)
      continue;
    if ((candidate->state == TASK_READY || candidate->state == TASK_RUNNING) && (candidate->priority < highestPriority))
    {
      highestPriority = candidate->priority;
      selectedIndex = (int32_t)nextIndex;
    /* Không break — tiếp tục để round-robin cùng priority */
    }
  }
  if (selectedIndex >= 0)
  {
    s_nextTCB = s_taskList[selectedIndex];
    s_currentTaskIndex = (uint32_t)selectedIndex;
  }
  else
  {
    /* Không có task READY — c  hạy idle (index 0) */
    s_nextTCB = s_taskList[0];
    s_currentTaskIndex = 0;
  }
  /* Cập nhật trạng thái */
  if (g_currentTCB != NULL && g_currentTCB->state == TASK_RUNNING)
  {
    g_currentTCB->state = TASK_READY;
  }
  if (s_nextTCB != NULL)
  {
    s_nextTCB->state = TASK_RUNNING;
  }
  /* Cập nhật g_currentTCB để Pe  ndSV handler có thể dùng */
  g_currentTCB = s_nextTCB;
}
/* ============================================================
 *  TDRTOS_TickHandler — Gọi từ SysTick_Handler
 * ============================================================ */

void TDRTOS_TickHandler(void)
{
  s_tickCount++;

  /* Cập nhật delay counter cho các task bị BLOCKED */
  for (uint32_t i = 0; i < s_taskCount; i++)
  {
    TCB_t *task = s_taskList[i];
    if (task == NULL)
      continue;

    if (task->state == TASK_BLOCKED)
    {
      if (task->delayTicks > 0)
      {
        task->delayTicks--;
        if (task->delayTicks == 0)
        {
          /* Hết thời gian delay → chuyển sang READY */
          task->state = TASK_READY;
        }
      }
    }
  }

  /* Chọn task tiếp theo và trigger PendSV */
  triggerContextSwitch();
}

/* ============================================================
 *  TDRTOS_Delay
 * ============================================================ */

void TDRTOS_Delay(uint32_t ticks)
{
  if (ticks == 0)
    return;
  if (g_currentTCB == NULL)
    return;

  TDRTOS_ENTER_CRITICAL();

  /* Block task hiện tại */
  g_currentTCB->state = TASK_BLOCKED;
  g_currentTCB->delayTicks = ticks;

  TDRTOS_EXIT_CRITICAL();

  /* Yield CPU ngay lập tức */
  triggerContextSwitch();

  /* Sau khi task được resume, tiếp tục từ đây */
}

/* ============================================================
 *  TDRTOS_GetTickCount
 * ============================================================ */

uint32_t TDRTOS_GetTickCount(void) { return s_tickCount; }

/* ============================================================
 *  TDRTOS_GetCurrentTCB
 * ============================================================ */

TCB_t *TDRTOS_GetCurrentTCB(void) { return (TCB_t *)g_currentTCB; }

/* ============================================================
 *  KHỞI ĐỘNG SysTick
 * ============================================================ */

static void startSysTick(void) {
  /* Tải Reload Value */
  SYSTICK_RVR = (TDRTOS_SYSTICK_LOAD - 1UL) & 0x00FFFFFFUL;

  /* Xóa Current Value */
  SYSTICK_CVR = 0UL;

  /* Enable SysTick với processor clock và interrupt */
  SYSTICK_CSR =
      SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT | SYSTICK_CSR_CLKSOURCE;
}

/* ============================================================
 *  TDRTOS_Start — Khởi động scheduler
 *
 *  Hàm này:
 *  1. Chọn task đầu tiên
 *  2. Thiết lập PSP từ stack của task đó
 *  3. Pop initial frame và nhảy vào task
 *  4. Bật SysTick
 *
 *  Sau đó scheduler chạy tự động thông qua SysTick + PendSV.
 * ============================================================ */

void TDRTOS_Start(void)
{
  /* Đảm bảo có ít nhất idle task */
  if (s_taskCount == 0)
    return;

  /* Chọn task đầu tiên */
  s_currentTaskIndex = 0;

  /* Tìm task READY có priority cao nhất để chạy đầu tiên */
  uint8_t highestPrio = TDRTOS_MAX_PRIORITY;
  uint32_t firstIdx = 0;
  for (uint32_t i = 0; i < s_taskCount; i++)
  {
    if (s_taskList[i] != NULL && s_taskList[i]->state == TASK_READY && s_taskList[i]->priority < highestPrio)
    {
      highestPrio = s_taskList[i]->priority;
      firstIdx = i;
    }
  }

  g_currentTCB = s_taskList[firstIdx];
  g_currentTCB->state = TASK_RUNNING;
  s_currentTaskIndex = firstIdx;
  s_schedulerRunning = 1;

  /* Khởi động SysTick */
  startSysTick();

  /*
   * Khởi động task đầu tiên bằng cách gọi SVC.
   * Cortex-M yêu cầu phải ở Handler Mode mới có thể dùng EXC_RETURN
   * để khôi phục context từ PSP. Nếu gọi BX 0xFFFFFFFD từ Thread Mode
   * sẽ gây lỗi UsageFault/HardFault (INVPC).
   */
  __asm volatile(
      /* Reset MSP về đỉnh stack (đọc từ Vector Table Offset Register) */
      "LDR     R0, =0xE000ED08        \n"
      "LDR     R0, [R0]               \n"
      "LDR     R0, [R0]               \n"
      "MSR     MSP, R0                \n"

      /* Enable interrupts */
      "CPSIE   I                      \n"
      "CPSIE   F                      \n"
      "DSB                            \n"
      "ISB                            \n"

      /* Kích hoạt Exception 11 (SVC) */
      "SVC     0                      \n"
      "NOP                            \n"
      "NOP                            \n" ::: "r0", "memory");

  /* KHÔNG BAO GIỜ đến đây */
  while (1) {
  }
}

/* ============================================================
 *  PendSV_Handler — Context Switch (Full Assembly)
 *
 *  Đây là trái tim của context switch.
 *  Attribute naked = không có function prologue/epilogue.
 *
 *  LUỒNG THỰC HIỆN:
 *  1. Hardware đã push {R0-R3,R12,LR,PC,xPSR} vào PSP stack
 *     (của task bị context switch ra)
 *  2. Ta push {R4-R11} vào PSP stack
 *  3. Lưu PSP vào g_currentTCB->sp
 *  4. g_currentTCB đã được cập nhật bởi TDRTOS_SelectNextTask()
 *  5. Load PSP từ g_currentTCB->sp (task mới)
 *  6. Pop {R4-R11} từ PSP stack mới
 *  7. BX LR → hardware pop {R0-R3,R12,LR,PC,xPSR} từ PSP mới
 *     → Task mới bắt đầu/tiếp tục chạy
 * ============================================================ */

__attribute__((naked)) void OS_PendSV_Handler(void)
{
  __asm volatile(
      /* ---- DISABLE INTERRUPTS (critical section) ---- */
      "CPSID   I                          \n"

      /* ---- SAVE CONTEXT CỦA TASK HIỆN TẠI ---- */
      /* Lấy PSP hiện tại (stack của task bị ngắt) */
      "MRS     R0, PSP                    \n"

      /* Kiểm tra xem task có dùng FPU không (bit 4 của LR == 0) */
      "TST     LR, #0x10                  \n"
      "IT      EQ                         \n"
      "VSTMDbeq R0!, {S16-S31}            \n"

      /* Push software-saved regs {R4-R11, LR} vào PSP stack */
      "STMDB   R0!, {R4-R11}          \n"

      /* Lưu PSP đã cập nhật vào g_currentTCB->sp (offset 0) */
      "LDR     R1, =g_currentTCB          \n" /* R1 = &g_currentTCB       */
      "LDR     R2, [R1]                   \n" /* R2 = g_currentTCB        */
      "CMP R2,#0 \n"
      "BEQ .\n"
      "STR     R0, [R2, #0]               \n" /* g_currentTCB->sp = PSP   */

      /* ---- CHỌN TASK TIẾP THEO ---- */
      /* PUSH LR để giữ lại giá trị EXC_RETURN của ngắt PendSV, PUSH R3 để căn
         chỉnh MSP 8-byte */
      "PUSH    {R3, LR}                   \n"
      "BL      TDRTOS_SelectNextTask      \n"
      "POP     {R3, LR}                   \n"

      /* ---- RESTORE CONTEXT CỦA TASK MỚI ---- */
      /* Load PSP của task mới từ g_currentTCB->sp */
      "LDR     R1, =g_currentTCB          \n" /* Lấy lại địa chỉ g_currentTCB */
      "LDR     R2, [R1]                   \n" /* R2 = g_currentTCB (mới)  */
      "LDR     R0, [R2, #0]               \n" /* R0 = new task's sp       */

      /* Pop software-saved regs {R4-R11, LR} từ PSP mới */
      "LDMIA   R0!, {R4-R11}          \n"

      /* Kiểm tra xem task có dùng FPU không */
      "TST     LR, #0x10                  \n"
      "IT      EQ                         \n"
      "VLDMIAeq R0!, {S16-S31}            \n"

      /* Cập nhật PSP với stack pointer mới */
      "MSR     PSP, R0                    \n"

      /* Đảm bảo pipeline flush trước khi enable interrupts */
      "ISB                                \n"

      /* ---- ENABLE INTERRUPTS ---- */
      "CPSIE   I                          \n"

      /*
       * Return từ exception với EXC_RETURN = LR:
       * Hardware tự pop {R0-R3,R12,LR,PC,xPSR} từ PSP
       * PC = địa chỉ task mới → task mới chạy
       */
      "BX      LR                         \n"

      ::
          : "memory");
}

/* ============================================================
 *  SysTick_Handler
 * ============================================================ */

void OS_SysTick_Handler(void)
{
  if (s_schedulerRunning)
  {
    TDRTOS_TickHandler();
  }
}
