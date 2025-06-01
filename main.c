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
uint32_t result=0;
uint8_t resultdisplay[8] = {0};
uint32_t num[8] = {0};
uint8_t sign[8] = {0};
uint8_t state=0;
uint32_t sum=0;
uint8_t numindex=0;
uint8_t signindex=0;
uint8_t caculatestate=0;
uint8_t warning_message[8]={0};
uint8_t warnsign=0;
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
void emptysign(uint8_t *);
void emptynum(uint32_t *);
void emptyall();
void continuecaculate();
void warning();
void check_sign();
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

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
    flag1 = 1;
    /* USER CODE BEGIN 3 */
    if (flag1 == 1) {
      flag1 = 0;
      I2C_ZLG7290_Read(&hi2c1, 0x71, 0x01, Rx1_Buffer, 1);
      swtich_key();
      if(warnsign==1)
      {
        warnsign=0;
        Tx1_Buffer[0] = 0x00;
        I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
      }
      I2C_ZLG7290_Read(&hi2c1, 0x71, 0x10, Rx2_Buffer, 8);
      check_sign();
      if(warnsign==1)
      {
        continue;
      }
      if(state==1)
      {
        for(int i=0;i<8;i++)
        {
          Rx2_Buffer[i] = 0x00;
        }
        I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Rx2_Buffer, 8);
        state=0;
      }
      if(caculatestate==1)
      {
        continuecaculate();state=0;
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

/* USER CODE BEGIN 4 */
void swtich_key(void) {
  switch (Rx1_Buffer[0]) {
  case 0x1C:
    flag = 1;
    break;
  case 0x1B:
    flag = 2;
    break;
  case 0x1A:
    flag = 3;
    break;
  case 0x14:
    flag = 4;
    break;
  case 0x13:
    flag = 5;
    break;
  case 0x12:
    flag = 6;
    break;
  case 0x0C:
    flag = 7;
    break;
  case 0x0B:
    flag = 8;
    break;
  case 0x0A:
    flag = 9;
    break;
  case 0x03:
    flag = 15;
    break;
  case 0x19:
    flag = 10;
    break;
  case 0x11:
    flag = 11;
    break;
  case 0x09:
    flag = 12;
    break;
  case 0x01:
    flag = 13;
    break;
  case 0x02:
    flag = 14;
    break;
  case 0x04:
    flag=16;
    break;
  default:
    break;
  }
}

void switch_flag(void) {
  switch (flag) {
  case 1:
    Tx1_Buffer[0] = 0x0c;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 2:
    Tx1_Buffer[0] = 0xDA;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 3:
    Tx1_Buffer[0] = 0xF2;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 4:
    Tx1_Buffer[0] = 0x66;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 5:
    Tx1_Buffer[0] = 0xB6;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 6:
    Tx1_Buffer[0] = 0xBE;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 7:
    Tx1_Buffer[0] = 0xE0;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 8:
    Tx1_Buffer[0] = 0xFE;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 9:
    Tx1_Buffer[0] = 0xE6;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 10:
    Tx1_Buffer[0] = 0x00;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
    Tx1_Buffer[0] = 0xEE;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 1);
    state=1;
    break;
  case 11:
    Tx1_Buffer[0] = 0x00;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
    Tx1_Buffer[0] = 0x3E;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 1);
    state=1;
    break;
  case 12:
    Tx1_Buffer[0] = 0x00;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
    Tx1_Buffer[0] = 0x9C;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 1);
    state=1;
    break;
  case 13:
    Tx1_Buffer[0] = 0x00;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
    Tx1_Buffer[0] = 0x7A;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 1);
    state=1;
    break;
  case 14:
    Tx1_Buffer[0] = 0x00;
    emptyall();
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, Tx1_Buffer, 8);
    break;
  case 15:
    Tx1_Buffer[0] = 0xFC;
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1 + memlen(Rx2_Buffer), Tx1_Buffer, 1);
    break;
  case 16:
    caculate();
    numdisplay(result);
    if(warnsign==1)
    {
      break;
    }
    I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, resultdisplay, 8);
    state=1;caculatestate=1;
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
void caculate()
{
  uint8_t i = 0;uint8_t numcount = memlen32(num);uint8_t signcount = memlen(sign);
  for (i = 0; i < signcount; ) {
      if (sign[i] == '*' || sign[i] == '/') {
          if (sign[i] == '*')
              result = num[i] * num[i + 1];
          else
          {
            if(num[i+1]==0)
            {
              emptyall();
              warning();
              warnsign=1;
              return;
            }
            else{
              result = num[i] / num[i + 1];
            }
          }
          // 更新数字数组
          num[i] = result;
          // 把后面的数字和运算符往前移
          for (uint8_t j = i + 1; j < numcount - 1; j++) {
              num[j] = num[j + 1];
          }
          for (uint8_t j = i; j < signcount - 1; j++) {
              sign[j] = sign[j + 1];
          }
          numcount--;
          signcount--;
          // 不自增i，因为当前位置可能还有乘除法
      } else {
            i++;
        }
    }
  // 然后处理加法和减法
  result = num[0];
  for (i = 0; i < signcount; i++) {
      if (sign[i] == '+')
          result += num[i + 1];
      else if (sign[i] == '-')
          result -= num[i + 1];
  }
}

void numdisplay(uint32_t result)
{
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
  if (result < 0 || result > 99999999) {
      // 超出范围时设为全 9
      warning();
      emptyall();
      warnsign=1;
      return;
  }
  // 存进临时数组
  uint8_t index = 0;
  uint8_t temp[8]={0};
  do {
      uint8_t digit = result % 10;
      temp[index++] = SEGMENT_CODES[digit];
      result /= 10;
  } while (result >= 1);
  uint8_t len=index;
  index-=1;
  //改变元素顺序存入结果数组
  for(uint8_t i =0;i<len;i++)
  {
    resultdisplay[i]=temp[index--];
  }
}
void storenumandsign()
{
  if((flag >= 1 && flag <= 9) || flag==15)
      {
        if(flag==15)
        {
          sum=sum*10;
        }
        else
        {
          sum=sum*10+flag;
        }
      }
      else if(flag >=10 && flag <= 13)
      {
        num[numindex++]=sum;
        sum=0;
        switch(flag)
        {
          case 10:
            sign[signindex++]='+';
            break;
          case 11:
            sign[signindex++]='-';
            break;
          case 12:
            sign[signindex++]='*';
            break;
          case 13:
            sign[signindex++]='/';
            break;
          default:
            break;
        }
      }
      else if(flag==16)
      {
        num[numindex++]=sum;
        sum=0;
      }
  
}
void emptysign(uint8_t *str)
{
  while(*str)
  {
    *str=0;
    str++;
  }
}
void emptynum(uint32_t *str)
{
  while(*str)
  {
    *str=0;
    str++;
  }
}
void emptyall()
{
  numindex=0;signindex=0;
  emptysign(sign);
  emptynum(num);
  emptysign(resultdisplay);
}
void continuecaculate()
{
  if((flag >= 1 && flag<=9 )|| flag ==15 )
  {
    emptyall();caculatestate=0;
  }
  else if( flag >= 10 && flag <= 13 )
  {
    emptyall();
    sum=result;caculatestate=0;result=0;
  }
}
void warning()
{
  for (int i = 0; i < 8; i++) {
          warning_message[i] = 0xFE;
      }
  I2C_ZLG7290_Write(&hi2c1, 0x70, ZLG_WRITE_ADDRESS1, warning_message, 8);
}
void check_sign()
{
  if(flag>=10&&flag<=13)
  {
    if(Rx2_Buffer[0]==0)
    {
      warning();
      emptyall();
      warnsign=1;
    }
    if(Rx2_Buffer[0]==0XEE ||Rx2_Buffer[0]==0X3E ||Rx2_Buffer[0]==0X9C ||Rx2_Buffer[0]==0X7A)
    {
      warning();
      emptyall();
      warnsign=1;
    }
  }
  else if(flag==16)
  {
    if(Rx2_Buffer[0]==0XEE ||Rx2_Buffer[0]==0X3E ||Rx2_Buffer[0]==0X9C ||Rx2_Buffer[0]==0X7A)
    {
      warning();
      emptyall();
      warnsign=1;
    }
  }
}