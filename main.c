/**
 ******************************************************************************
 * File Name          : main.c
 * Description        : Main program body
 ******************************************************************************
 *
 * COPYRIGHT(c) 2015 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *notice, this list of conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "usart.h"

/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "zlg7290.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define ZLG_READ_ADDRESS1 0x01
#define ZLG_READ_ADDRESS2 0x10
#define ZLG_WRITE_ADDRESS1 0x10
#define ZLG_WRITE_ADDRESS2 0x11
#define BUFFER_SIZE1 (countof(Tx1_Buffer))
#define BUFFER_SIZE2 (countof(Rx2_Buffer))
#define countof(a) (sizeof(a) / sizeof(*(a)))

uint8_t flag;
uint8_t flag1 = 0;
uint8_t Rx2_Buffer[8] = {0};
uint8_t Tx1_Buffer[8] = {0};
uint8_t Rx1_Buffer[1] = {0};
uint32_t result = 0;
uint8_t resultdisplay[8] = {0};
uint32_t num[8] = {0};
uint8_t sign[8] = {0};
uint8_t state = 0;
uint32_t sum = 0;
uint8_t numindex = 0;
uint8_t signindex = 0;
uint8_t caculatestate = 0;
uint8_t warning_message[8] = {0};
uint8_t warnsign = 0;

uint8_t rx2flag_map[0x1D] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void swtich_key(void);
void switch_flag(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
uint8_t memlen(uint8_t *);
uint8_t memlen32(uint32_t *);
void caculate();
void numdisplay(uint32_t result);
void storenumandsign();
void clearsign(uint8_t *);
void clearnum(uint32_t *);
void clearall();
void continuecaculate();
void warning();
void init_map();
int is_legal_input();
uint8_t validate_map();

// 新增数组越界检查宏
#define CHECK_ARRAY_BOUND(index, max_size) \
    if ((index) >= (max_size)) { \
        clearall(); \
        warning(); \
        return; \
    }
/* USER CODE END 0 */

int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU
   * Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  printf("\n\r");
  printf("\n\r-------------------------------------------------\r\n");
  printf("\n\r FS-STM32\r\n");
  /* USER CODE END 2 */

  init_map();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
    flag1 = 1;
    /* USER CODE BEGIN 3 */
    if (flag1 == 1) {
      while (!validate_map())
        init_map();
      flag1 = 0;
      I2C_ZLG7290_Read(&hi2c1, 0x71, 0x01, Rx1_Buffer, 1);
      flag = rx2flag_map[Rx1_Buffer[0]];
      if (warnsign == 1) {
        warnsign = 0;
        Tx1_Buffer[0] = 0x00;
        I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
      }
      I2C_ZLG7290_Read(&hi2c1, 0x71, 0x10, Rx2_Buffer, 8);
      if (!is_legal_input()) {
        clearall();
        warning();
        warnsign = 1;
        continue;
      }

      // 上一轮我们输入了符号， 这一轮读取输入之后要先清空数码管
      if (state == 1) {
        for (int i = 0; i < 8; i++) {
          Rx2_Buffer[i] = 0x00;
        }
        I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Rx2_Buffer, 8);
        state = 0;
      }

      // 上一轮我们输入了=号
      if (caculatestate == 1) {
        continuecaculate();
        state = 0;
      }
      storenumandsign();
      switch_flag();
    }
  }
  /* USER CODE END 3 */
}

/** System Clock Configuration
 */
void SystemClock_Config(void) {

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

// 数码管显示相应的字符的参数
uint8_t flag2tx1_map[16] = {
    0 /* not used*/,
    0x0c, // index -> 1
    0xDA, // index -> 2
    0xF2, // index -> 3
    0x66, // index -> 4
    0xB6, // index -> 5
    0xBE, // index -> 6
    0xE0, // index -> 7
    0xFE, // index -> 8
    0xE6, // index -> 9
    0xEE, // index -> 10 加号
    0x3E, // index -> 11 减号
    0x9C, // index -> 12 乘号
    0x7A, // index -> 13 除号
    0x00, // index -> 14 清零符号
    0xFC, // index -> 15 数字0
};

void switch_flag(void) {
  switch (flag) {
// 处理输入数字1-9的情况
#define XX(A)                                                                  \
  case A:                                                                      \
    Tx1_Buffer[0] = flag2tx1_map[A];                                           \
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer),   \
                      Tx1_Buffer, 1);                                          \
    break // 这一行不要删除
    XX(1);
    XX(2);
    XX(3);
    XX(4);
    XX(5);
    XX(6);
    XX(7);
    XX(8);
    XX(9);

// 处理运算符号的情况（不包括等号和清零符号）
#define YY(A)                                                                  \
  case A:                                                                      \
    Tx1_Buffer[0] = 0x00;                                                      \
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);        \
    Tx1_Buffer[0] = flag2tx1_map[A];                                           \
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 1);        \
    state = 1;                                                                 \
    break // 这一行不要删除
    YY(10);
    YY(11);
    YY(12);
    YY(13);

  case 14: // 清零符号
    Tx1_Buffer[0] = 0x00;
    clearall();
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
    break;

    // 处理数字0
    XX(15);

  case 16: // 等于号
    caculate();
    numdisplay(result);
    if (warnsign == 1) {
      break;
    }
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, resultdisplay, 8);
    state = 1;
    caculatestate = 1;
    break;
  default:
    break;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) { flag1 = 1; }

int fputc(int ch, FILE *f) {
  uint8_t tmp[1] = {0};
  tmp[0] = (uint8_t)ch;
  HAL_UART_Transmit(&huart1, tmp, 1, 10);
  return ch;
}

uint8_t memlen(uint8_t *str) {
  int count = 0;
  while (*str++)
    count++;
  return count;
}

uint8_t memlen32(uint32_t *str) {
  int count = 0;
  while (*str++)
    count++;
  return count;
}

void caculate() {
  uint8_t i = 0;
  uint8_t numcount = memlen32(num);
  uint8_t signcount = memlen(sign);

  // 先处理乘除法
  for (i = 0; i < signcount;) {
    if (sign[i] == '*' || sign[i] == '/') {
      // 除数为零检查（已存在，增强提示）
      if (sign[i] == '/' && num[i + 1] == 0) {
        clearall();
        warning(); // 显示错误
        warnsign = 1;
        return;
      }

      // 乘法溢出检查（无符号数溢出检测）
      if (sign[i] == '*') {
        uint64_t temp = (uint64_t)num[i] * num[i + 1];
        if (temp > UINT32_MAX) { // 超过32位无符号数范围
          clearall();
          warning();
          warnsign = 1;
          return;
        }
        result = (uint32_t)temp;
      } else {
        result = num[i] / num[i + 1];
      }

      // 更新数组
      num[i] = result;
      for (uint8_t j = i + 1; j < numcount - 1; j++) {
        num[j] = num[j + 1];
      }
      for (uint8_t j = i; j < signcount - 1; j++) {
        sign[j] = sign[j + 1];
      }
      numcount--;
      signcount--;
    } else {
      i++;
    }
  }

  // 处理加减法时的溢出检查
  result = num[0];
  for (i = 0; i < signcount; i++) {
    uint32_t operand = num[i + 1];
    if (sign[i] == '+') {
      // 加法溢出检查
      if (result > UINT32_MAX - operand) {
        clearall();
        warning();
        warnsign = 1;
        return;
      }
      result += operand;
    } else {
      // 减法不做负数处理（无符号数特性）
      result -= operand;
    }
  }

  // 结果范围检查（0-99999999）
  if (result > 99999999) {
    clearall();
    warning();
    warnsign = 1;
    return;
  }
}

void numdisplay(uint32_t result) {
  const uint8_t SEGMENT_CODES[10] = {
      0xFC, // 0
      0x0C, // 1
      0xDA, // 2
      0xF2, // 3
      0x66, // 4
      0xB6, // 5
      0xBE, // 6
      0xE0, // 7
      0xFE, // 8
      0xE6  // 9
  };

  // 结果合法性检查（已在caculate中处理，此处保留冗余检查）
  if (result > 99999999) {
    warning();
    clearall();
    warnsign = 1;
    return;
  }

  // 原有显示逻辑
  uint8_t index = 0;
  uint8_t temp[8] = {0};
  do {
    uint8_t digit = result % 10;
    temp[index++] = SEGMENT_CODES[digit];
    result /= 10;
  } while (result >= 1);
  
  uint8_t len = index;
  index -= 1;
  for (uint8_t i = 0; i < len; i++) {
    resultdisplay[i] = temp[index--];
  }
}

void storenumandsign() {
    // 新增数组越界预防
    if ((flag >= 1 && flag <= 9) || flag == 15) {
        // 数值输入时检查是否超过8位
        if (numindex >= 8) {
            clearall();
            warning();
            return;
        }
        if (flag == 15) {
            sum = sum * 10;
        } else {
            sum = sum * 10 + flag;
        }
        // 每个数字不能大于数码管显示的极限
        printf("sum = %u\n", sum);
        if (sum > 99999999) {
          clearall();
          warning();
          return ;
        }
    } else if (flag >= 10 && flag <= 13) {
        // 操作数存储越界检查
        CHECK_ARRAY_BOUND(numindex, 7); // 最多存7个操作数
        num[numindex++] = sum;
        sum = 0;
        
        // 运算符存储越界检查
        CHECK_ARRAY_BOUND(signindex, 7); // 最多存7个运算符
        switch (flag) {
            case 10:
                sign[signindex++] = '+';
                break;
            case 11:
                sign[signindex++] = '-';
                break;
            case 12:
                sign[signindex++] = '*';
                break;
            case 13:
                sign[signindex++] = '/';
                break;
            default:
                break;
        }
    } else if (flag == 16) {
        // 等号处理时检查操作数数量
        if (numindex >= 7) { // 防止最后一个操作数存储越界
            clearall();
            warning();
            return;
        }
        num[numindex++] = sum;
        sum = 0;
    }

}

void clearsign(uint8_t *str) {
  while (*str) {
    *str = 0;
    str++;
  }
}

void clearnum(uint32_t *str) {
  while (*str) {
    *str = 0;
    str++;
  }
}

void clearall() {
  numindex = 0;
  signindex = 0;
  clearsign(sign);
  clearnum(num);
  clearsign(resultdisplay);
}

void continuecaculate() {
  clearall();
  caculatestate = 0;

  // 继续计算时检查操作数存储越界
  if (flag >= 10 && flag <= 13) {
    CHECK_ARRAY_BOUND(numindex, 7);
    sum = result;
    result = 0;
  }
}

void warning() {
  for (int i = 0; i < 8; i++) {
    warning_message[i] = 0xFE;
  }
  I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, warning_message, 8);
}

static inline uint8_t is_operator(uint8_t _flag) {
  return flag >= 10 && flag <= 13;
}
static inline uint8_t buffer_is_operator(uint8_t buf[]) {
  return buf[0] == 0XEE || buf[0] == 0X3E || buf[0] == 0X9C || buf[0] == 0X7A;
}

int is_legal_input() {
    // 增强输入合法性校验
    static uint8_t last_input = 0; // 记录上一次输入类型：0=无，1=数字，2=运算符，3=等号
    
    // 处理特殊按键
    if (flag == 14) { // 清零键
        last_input = 0;
        return 1;
    } else if (flag == 16) { // 等号
        if (last_input != 1) { // 等号前必须是数字
            last_input = 0;
            return 0;
        }
        last_input = 3;
        return 1;
    }

    // 判断当前输入类型
    uint8_t current_type = 0;
    if ((flag >= 1 && flag <= 9) || flag == 15) {
        current_type = 1; // 数字
    } else if (flag >= 10 && flag <= 13) {
        current_type = 2; // 运算符
    } else {
        return 1; // 其他按键忽略
    }

    // 校验逻辑
    if (last_input == 0) { // 初始状态
        if (current_type == 2) { // 不能以运算符开头
            return 0;
        }
    } else if (last_input == 2) { // 上一次是运算符
        if (current_type == 2) { // 不能连续运算符
            return 0;
        }
    } else if (last_input == 3) { // 上一次是等号
        if (current_type == 1) {
          clearall();
        }
    }

    last_input = current_type;
    return 1;
}

void init_map() {
  rx2flag_map[0x1C] = 1;
  rx2flag_map[0x1B] = 2;
  rx2flag_map[0x1A] = 3;
  rx2flag_map[0x14] = 4;
  rx2flag_map[0x13] = 5;
  rx2flag_map[0x12] = 6;
  rx2flag_map[0x0C] = 7;
  rx2flag_map[0x0B] = 8;
  rx2flag_map[0x0A] = 9;
  rx2flag_map[0x03] = 15;
  rx2flag_map[0x19] = 10;
  rx2flag_map[0x11] = 11;
  rx2flag_map[0x09] = 12;
  rx2flag_map[0x01] = 13;
  rx2flag_map[0x02] = 14;
  rx2flag_map[0x04] = 16;
}

uint8_t validate_map() {
  if (!(rx2flag_map[0x1C] == 1))
    return 0;
  if (!(rx2flag_map[0x1B] == 2))
    return 0;
  if (!(rx2flag_map[0x1A] == 3))
    return 0;
  if (!(rx2flag_map[0x14] == 4))
    return 0;
  if (!(rx2flag_map[0x13] == 5))
    return 0;
  if (!(rx2flag_map[0x12] == 6))
    return 0;
  if (!(rx2flag_map[0x0C] == 7))
    return 0;
  if (!(rx2flag_map[0x0B] == 8))
    return 0;
  if (!(rx2flag_map[0x0A] == 9))
    return 0;
  if (!(rx2flag_map[0x03] == 15))
    return 0;
  if (!(rx2flag_map[0x19] == 10))
    return 0;
  if (!(rx2flag_map[0x11] == 11))
    return 0;
  if (!(rx2flag_map[0x09] == 12))
    return 0;
  if (!(rx2flag_map[0x01] == 13))
    return 0;
  if (!(rx2flag_map[0x02] == 14))
    return 0;
  if (!(rx2flag_map[0x04] == 16))
    return 0;
  return 1;
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
    number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
    line) */
  /* USER CODE END 6 */
}

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/