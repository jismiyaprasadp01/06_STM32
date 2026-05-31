
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

TaskHandle_t xTaskHandleM = NULL;
TaskHandle_t xTaskHandleE = NULL;

// Macros
#define TRUE          1
#define FALSE         0

UART_HandleTypeDef huart2;


char user_msg[100];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

static void prvSetupHardware(void);
static void prvSetupUart(void);
static void prvSetupGPIO(void);
void EmployeeDoWork(unsigned char tickedId);
// Task to be created
static void vManagerTask(void *params);
static void vEmployeeTask(void *params);

void rtos_delay(uint32_t delay_in_ms);

// Semaphore handle variable
xSemaphoreHandle xWork;

// Queue handle
xQueueHandle xWorkQueue;

void printmsg(char *msg)
{
	for(uint32_t i = 0;i < strlen(msg);i++)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)&msg[i], 1, 100);
	}
}


int main(void)
{

  HAL_Init();
  SystemClock_Config();

  prvSetupHardware();

  sprintf(user_msg,"Demo of Binary semaphore usage between 2 tasks\n\r");
  printmsg(user_msg);

 // creating a binary semaphore
  vSemaphoreCreateBinary(xWork);

  // create queue
  xWorkQueue = xQueueCreate(1,sizeof(unsigned int));

  if((xWork != NULL) && (xWorkQueue != NULL))
  {
	  // create the manager task
	  xTaskCreate(vManagerTask,"Manager",500,NULL,3,NULL);

	  // create employee task
	  xTaskCreate(vEmployeeTask,"Employee",500,NULL,1,NULL);

	  vTaskStartScheduler();
  }
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void vManagerTask(void *params)
{
	unsigned int xWorkTicketId;
	portBASE_TYPE xStatus;

	xSemaphoreGive(xWork);

	while(1)
	{
		// random ticket numbers
		xWorkTicketId = (rand() & 0x1ff);

		// send ticket to queue
		xStatus = xQueueSend(xWorkQueue,&xWorkTicketId,portMAX_DELAY);

		if(xStatus != pdPASS)
		{
			sprintf(user_msg,"Could not send to the queue\n\r");
			printmsg(user_msg);
		}
		else
		{
			// manager notifying employee by giving the semaphore
			xSemaphoreGive(xWork);
			taskYIELD();
		}
	}
}


void vEmployeeTask(void *params)
{
	unsigned char xWorkTicketId;
	portBASE_TYPE xStatus;

	while(1)
	{
		// take the semaphore
		xSemaphoreTake(xWork,0);

		//get the ticket id
		xStatus = xQueueReceive(xWorkQueue, &xWorkTicketId, 0);

		if(xStatus == pdPASS)
		{
			EmployeeDoWork(xWorkTicketId);
		}
		else
		{
			sprintf(user_msg,"Employee task: Queue is empty, nothing to do. \r\n");
			printmsg(user_msg);
		}
	}
}

void EmployeeDoWork(unsigned char tickedId)
{
	sprintf(user_msg,"Employee task: Working on Ticket id: %d\r\n",tickedId);
	printmsg(user_msg);
	vTaskDelay(tickedId);
}

void rtos_delay(uint32_t delay_in_ms)
{
	uint32_t current_tick_count = xTaskGetTickCount();

	uint32_t delay_in_ticks = (delay_in_ms * configTICK_RATE_HZ) / 1000;

	while(xTaskGetTickCount() < (current_tick_count + delay_in_ticks));
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
	// setup LED and Button
	prvSetupGPIO();
	// setup UART
	prvSetupUart();
}

static void prvSetupGPIO(void)
{
	 GPIO_InitTypeDef GPIO_InitStruct = {0};

	 __HAL_RCC_GPIOA_CLK_ENABLE();
	 __HAL_RCC_GPIOD_CLK_ENABLE();

	 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

	  /*Configure GPIO pin : PA0 */
	  GPIO_InitStruct.Pin = GPIO_PIN_0;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pin : PD12 */
	  GPIO_InitStruct.Pin = GPIO_PIN_12;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
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
