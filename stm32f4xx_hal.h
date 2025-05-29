#include <stdint.h>
#include <stdio.h>

int huart1;

void HAL_Init() {
  // TODO
}

// 假设你不包含 HAL 官方头文件，手动定义结构体和宏
typedef struct {
  uint32_t PLLState;
  uint32_t PLLSource;
  uint32_t PLLM;
  uint32_t PLLN;
  uint32_t PLLP;
  uint32_t PLLQ;
} RCC_PLLInitTypeDef;

typedef struct {
  uint32_t OscillatorType;
  uint32_t HSEState;
  RCC_PLLInitTypeDef PLL;
} RCC_OscInitTypeDef;

typedef struct {
  uint32_t ClockType;
  uint32_t SYSCLKSource;
  uint32_t AHBCLKDivider;
  uint32_t APB1CLKDivider;
  uint32_t APB2CLKDivider;
} RCC_ClkInitTypeDef;

#define RCC_OSCILLATORTYPE_HSE 1
#define RCC_HSE_ON 1
#define RCC_PLL_ON 1
#define RCC_PLLSOURCE_HSE 1
#define RCC_PLLP_DIV2 1
#define RCC_CLOCKTYPE_SYSCLK 1
#define RCC_CLOCKTYPE_PCLK1 2
#define RCC_CLOCKTYPE_PCLK2 4
#define RCC_SYSCLKSOURCE_PLLCLK 1
#define RCC_SYSCLK_DIV1 1
#define RCC_HCLK_DIV4 4
#define RCC_HCLK_DIV2 2
#define FLASH_LATENCY_5 5
#define SYSTICK_CLKSOURCE_HCLK 1

#define __PWR_CLK_ENABLE() (void)0
#define __HAL_PWR_VOLTAGESCALING_CONFIG(x) (void)0
#define HAL_RCC_OscConfig(x) (void)0
#define HAL_RCC_ClockConfig(x, y) (void)0
#define HAL_SYSTICK_Config(x) (void)0
#define HAL_SYSTICK_CLKSourceConfig(x) (void)0
#define HAL_NVIC_SetPriority(irq, prio, subprio) (void)0

// 空实现 HAL_UART_Transmit，只打印字符
int HAL_UART_Transmit(int *huart, uint8_t *pData, uint16_t Size,
                      uint32_t Timeout) {
  return 0;
}