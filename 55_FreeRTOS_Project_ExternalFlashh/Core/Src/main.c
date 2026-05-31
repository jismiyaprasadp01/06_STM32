
#include "main.h"
#include "cmsis_os.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include <string.h>


SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* Data Structure */
typedef struct
{
    uint32_t timestamp;
    int distance;
} DistanceData_t;

/* Queues */
QueueHandle_t alertQueue;
QueueHandle_t uartQueue;
QueueHandle_t flashQueue;

/* Task Handles */
TaskHandle_t SensorTaskHandle;
TaskHandle_t AlertTaskHandle;
TaskHandle_t UARTTaskHandle;
TaskHandle_t FlashTaskHandle;


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);

void SensorTask(void *argument);
void AlertTask(void *argument);
void UARTTask(void *argument);
void FlashTask(void *argument);

void Error_Handler(void);

/* Flash Functions */
void W25Q_WriteEnable(void);
void W25Q_SectorErase(uint32_t addr);
void W25Q_WriteData(uint32_t addr, uint8_t *data, uint16_t size);
void W25Q_ReadData(uint32_t addr, uint8_t *data, uint16_t size);

/* Flash Variables */
uint32_t flashAddress = 0;
uint32_t recordCount = 0;

/* Flash CS Macros */
#define FLASH_CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define FLASH_CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

/* Microsecond Delay */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

/* HC-SR04 Read */
float HCSR04_Read(void)
{
    uint32_t time = 0;
    uint32_t timeout;

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

    timeout = 30000;

    while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)
    {
        if (--timeout == 0)
            return -1;
    }

    __HAL_TIM_SET_COUNTER(&htim2, 0);

    timeout = 30000;

    while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET)
    {
        if (--timeout == 0)
            return -1;
    }

    time = __HAL_TIM_GET_COUNTER(&htim2);

    return (float)time / 58.0f;
}

/* ================= FLASH DRIVER ================= */

void W25Q_WriteEnable(void)
{
    uint8_t cmd = 0x06;

    FLASH_CS_LOW();

    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);

    FLASH_CS_HIGH();
}

void W25Q_SectorErase(uint32_t addr)
{
    uint8_t cmd[4];

    W25Q_WriteEnable();

    cmd[0] = 0x20;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    FLASH_CS_LOW();

    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);

    FLASH_CS_HIGH();

    HAL_Delay(100);
}

void W25Q_WriteData(uint32_t addr, uint8_t *data, uint16_t size)
{
    uint8_t cmd[4];

    W25Q_WriteEnable();

    cmd[0] = 0x02;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    FLASH_CS_LOW();

    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);

    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);

    FLASH_CS_HIGH();

    HAL_Delay(5);
}

void W25Q_ReadData(uint32_t addr, uint8_t *data, uint16_t size)
{
    uint8_t cmd[4];

    cmd[0] = 0x03;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    FLASH_CS_LOW();

    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);

    HAL_SPI_Receive(&hspi1, data, size, HAL_MAX_DELAY);

    FLASH_CS_HIGH();
}

/* ================= SENSOR TASK ================= */
void SensorTask(void *argument)
{
    DistanceData_t data;

    while (1)
    {
        data.timestamp = xTaskGetTickCount();

        data.distance = (int)HCSR04_Read();

        if (data.distance >= 0)
        {
            xQueueSend(alertQueue, &data, portMAX_DELAY);

            xQueueSend(uartQueue, &data, portMAX_DELAY);

            xQueueSend(flashQueue, &data, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ================= ALERT TASK ================= */

void AlertTask(void *argument)
{
    DistanceData_t data;

    while (1)
    {
        if (xQueueReceive(alertQueue,
                          &data,
                          portMAX_DELAY) == pdPASS)
        {
            if (data.distance < 30)
            {
                HAL_GPIO_WritePin(GPIOD,
                                  GPIO_PIN_12,
                                  GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOD,
                                  GPIO_PIN_12,
                                  GPIO_PIN_RESET);
            }
        }
    }
}

/* ================= UART TASK ================= */

void UARTTask(void *argument)
{
    DistanceData_t data;

    char msg[150];

    while (1)
    {
        if (xQueueReceive(uartQueue,
                          &data,
                          portMAX_DELAY) == pdPASS)
        {
            sprintf(msg,
                    "\r\n==============================\r\n"
                    "TIME      : %lu ms\r\n"
                    "DISTANCE  : %d cm\r\n",
                    data.timestamp,
                    data.distance);

            HAL_UART_Transmit(&huart2,
                              (uint8_t*)msg,
                              strlen(msg),
                              HAL_MAX_DELAY);

            if (data.distance < 30)
            {
                sprintf(msg,
                        "STATUS    : ALERT! OBJECT CLOSE\r\n");
            }
            else
            {
                sprintf(msg,
                        "STATUS    : SAFE\r\n");
            }

            HAL_UART_Transmit(&huart2,
                              (uint8_t*)msg,
                              strlen(msg),
                              HAL_MAX_DELAY);

            sprintf(msg,
                    "==============================\r\n");

            HAL_UART_Transmit(&huart2,
                              (uint8_t*)msg,
                              strlen(msg),
                              HAL_MAX_DELAY);
        }
    }
}

/* ================= FLASH TASK ================= */

void FlashTask(void *argument)
{
    DistanceData_t data;
    DistanceData_t readData;

    char msg[100];

    W25Q_SectorErase(0);

    while (1)
    {
        if (xQueueReceive(flashQueue,
                          &data,
                          portMAX_DELAY) == pdPASS)
        {
            W25Q_WriteData(flashAddress,
                           (uint8_t *)&data,
                           sizeof(DistanceData_t));

            sprintf(msg,
                    "FLASH LOGGED -> Addr:%lu Dist:%d\r\n",
                    flashAddress,
                    data.distance);

            HAL_UART_Transmit(&huart2,
                              (uint8_t *)msg,
                              strlen(msg),
                              HAL_MAX_DELAY);

            recordCount++;

            if(recordCount % 5 == 0)
            {
                sprintf(msg,
                        "\r\n===== FLASH VERIFY =====\r\n");

                HAL_UART_Transmit(&huart2,
                                  (uint8_t *)msg,
                                  strlen(msg),
                                  HAL_MAX_DELAY);

                uint32_t readAddr = 0;

                for(int i = 0; i < 5; i++)
                {
                    W25Q_ReadData(readAddr,
                                  (uint8_t *)&readData,
                                  sizeof(DistanceData_t));

                    sprintf(msg,
                            "TIME:%lu DIST:%d\r\n",
                            readData.timestamp,
                            readData.distance);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t *)msg,
                                      strlen(msg),
                                      HAL_MAX_DELAY);

                    readAddr += sizeof(DistanceData_t);
                }
            }

            flashAddress += sizeof(DistanceData_t);
        }
    }
}

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();

  HAL_TIM_Base_Start(&htim2);

      HAL_UART_Transmit(&huart2,
                        (uint8_t*)"System Start\r\n",
                        15,
                        HAL_MAX_DELAY);

      /* Queue Creation */

          alertQueue = xQueueCreate(5,
                                    sizeof(DistanceData_t));

          uartQueue = xQueueCreate(5,
                                   sizeof(DistanceData_t));

          flashQueue = xQueueCreate(5,
                                    sizeof(DistanceData_t));
          /* Task Creation */

              xTaskCreate(SensorTask,
                          "SensorTask",
                          256,
                          NULL,
                          4,
                          &SensorTaskHandle);

              xTaskCreate(AlertTask,
                          "AlertTask",
                          256,
                          NULL,
                          3,
                          &AlertTaskHandle);

              xTaskCreate(FlashTask,
                          "FlashTask",
                          256,
                          NULL,
                          2,
                          &FlashTaskHandle);

              xTaskCreate(UARTTask,
                          "UARTTask",
                          256,
                          NULL,
                          1,
                          &UARTTaskHandle);

              vTaskStartScheduler();

  while (1)
  {

  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA1 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
