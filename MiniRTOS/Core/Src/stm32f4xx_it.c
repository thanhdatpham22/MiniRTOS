/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1) {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  volatile uint32_t psp;

  __asm volatile("MRS %0, PSP" : "=r"(psp));

  volatile uint32_t msp;

  __asm volatile("MRS %0, MSP" : "=r"(msp));
  volatile uint32_t cfsr = SCB->CFSR;
  volatile uint32_t hfsr = SCB->HFSR;
  volatile uint32_t mmfar = SCB->MMFAR;
  volatile uint32_t bfar = SCB->BFAR;
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
__attribute__((naked))
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */
 /*
 * Khôi phục context của task đầu tiên và nhảy vào nó.
 * Ở đây ta đang trong Handler Mode nên có thể dùng EXC_RETURN.
 */
    __asm volatile (
        "LDR     R3, =g_currentTCB      \n"
        "LDR     R1, [R3]               \n"
        "LDR     R0, [R1]               \n"   /* R0 = tcb->sp */

        "LDMIA   R0!, {R4-R11}          \n"   /* Pop software registers */
        "MSR     PSP, R0                \n"   /* Cập nhật PSP */

        "ISB                            \n"
        "LDR     LR, =0xFFFFFFFD        \n"   /* Trở v�? Thread Mode dùng PSP */
        "BX      LR                     \n"
    );
  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
__attribute__((naked))
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */
//  OS_PendSV_Handler();
	__asm volatile(
	  /* ---- DISABLE INTERRUPTS (critical section) ---- */
	  "CPSID   I                          \n"

	  /* ---- SAVE CONTEXT CỦA TASK HIỆN TẠI ---- */
	  /* Lấy PSP hiện tại (stack của task bị ngắt) */
	  "MRS     R0, PSP                    \n"

	  /* Kiểm tra xem task có dùng FPU không (bit 4 của LR == 0) */
//	  "TST     LR, #0x10                  \n"
//	  "IT      EQ                         \n"
//	  "VSTMDbeq R0!, {S16-S31}            \n"

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
//	  "TST     LR, #0x10                  \n"
//	  "IT      EQ                         \n"
//	  "VLDMIAeq R0!, {S16-S31}            \n"

	  /* Cập nhật PSP với stack pointer mới */
	  "MSR     PSP, R0                    \n"

	  /* �?ảm bảo pipeline flush trước khi enable interrupts */
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
  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  OS_SysTick_Handler();
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles PVD interrupt through EXTI line 16.
  */
void PVD_IRQHandler(void)
{
  /* USER CODE BEGIN PVD_IRQn 0 */

  /* USER CODE END PVD_IRQn 0 */
  HAL_PWR_PVD_IRQHandler();
  /* USER CODE BEGIN PVD_IRQn 1 */

  /* USER CODE END PVD_IRQn 1 */
}

/**
  * @brief This function handles Flash global interrupt.
  */
void FLASH_IRQHandler(void)
{
  /* USER CODE BEGIN FLASH_IRQn 0 */

  /* USER CODE END FLASH_IRQn 0 */
  HAL_FLASH_IRQHandler();
  /* USER CODE BEGIN FLASH_IRQn 1 */

  /* USER CODE END FLASH_IRQn 1 */
}

/**
  * @brief This function handles RCC global interrupt.
  */
void RCC_IRQHandler(void)
{
  /* USER CODE BEGIN RCC_IRQn 0 */

  /* USER CODE END RCC_IRQn 0 */
  /* USER CODE BEGIN RCC_IRQn 1 */

  /* USER CODE END RCC_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
