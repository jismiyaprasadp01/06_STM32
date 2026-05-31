
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


UART_HandleTypeDef huart2;
char user_msg[100];


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

static void prvSetupHardware(void);
static void prvSetupUart(void);

void vTask1(void *params);
void vTask2(void *params);

void printmsg(char *msg)
{
	for(uint32_t i = 0;i < strlen(msg);i++)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)&msg[i], 1, 100);
	}
}

// mutex handle
SemaphoreHandle_t xBinSemaphore;

int main(void)
{

  HAL_Init();
  SystemClock_Config();

  prvSetupHardware();

  sprintf(user_msg,"Demo of mutual exclusion using binary semaphore \r\n");
  printmsg(user_msg);

  xBinSemaphore = xSemaphoreCreateMutex();

  if(xBinSemaphore != NULL)
  {
	  xTaskCreate(vTask1, "Task1", 500, NULL, 1, NULL);

	  xTaskCreate(vTask2, "Task2", 500, NULL, 1, NULL);

	  vTaskStartScheduler();
  }
  else
  {
	  sprintf(user_msg,"Binary semaphore creation failed\r\n");
	  printmsg(user_msg);
  }
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void vTask1(void *params)
{
	const char *pcTaskName = "Task 1 is running\r\n";
	while(1)
	{
		// take semaphore
		xSemaphoreTake(xBinSemaphore,portMAX_DELAY);
		sprintf(user_msg,"%s",pcTaskName);
		printmsg(user_msg);

		// give semaphore
		xSemaphoreGive(xBinSemaphore);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}


void vTask2(void *params)
{
	const char *pcTaskName = "Task 2 is running\r\n";
	while(1)
	{
		// take semaphore
		xSemaphoreTake(xBinSemaphore,portMAX_DELAY);
		sprintf(user_msg,"%s",pcTaskName);
		printmsg(user_msg);

		// give semaphore
		xSemaphoreGive(xBinSemaphore);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}


static void prvSetupUart(void)
{
	// enable the uart2 peripheral clock
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//PA2 is UART2_TX, PA3 is UART_RX
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}



static void prvSetupHardware(void)
{
	// setup UART
	prvSetupUart();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
#ifdef USE_FULL_ASSERT
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
