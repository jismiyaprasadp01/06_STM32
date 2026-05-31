
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

// Macros
#define TRUE               1
#define FALSE              0

#define LED_ON_COMMAND     1
#define LED_OFF_COMMAND    2
#define LED_TOGGLE_COMMAND 3
#define LED_TOGGLE_STOP    4
#define LED_READ_STATUS    5

// Command Format
typedef struct APP_CMD{
	uint8_t COMMAND_NUM;
	uint8_t COMMAND_ARGS[10];
}APP_CMD_t;

// Private function prototypes
void SystemClock_Config(void);
static void prvSetupHardware(void);
static void prvSetupUart(void);
static void prvSetupGPIO(void);
void rtos_delay(uint32_t delay_in_ms);

uint8_t getCommandCode(uint8_t *buffer);
void getArguments(uint8_t *buffer);

// Prototypes of command helper functions
void make_led_on(void);
void make_led_off(void);
void led_toggle_start(uint32_t duration);
void led_toggle_stop(void);
void read_led_status(void);
void print_error_message(char* task_msg);

void led_toggle(TimerHandle_t xIimer);

// Tasks prototypes
void vTask1_menu_display(void *params);
void vTask2_cmd_handling(void *params);
void vTask3_cmd_processing(void *params);
void vTask4_uart_write(void *params);

// Global space for some variables
UART_HandleTypeDef huart2;
char user_msg[100] = {0};
uint8_t command_buffer[20];
uint8_t command_len = 0;
uint8_t rx_byte;

// Menu
char menu[] = {"\
	\r\nLED_ON              ---> 1 \
	\r\nLED_OFF             ---> 2 \
	\r\nLED_TOGGLE          ---> 3 \
	\r\nLED_TOGGLE_OFF      ---> 4 \
	\r\nLED_READ_STATUS     ---> 5 \
	\r\nEXIT_APP            ---> 0 \
	\r\nType your option here: "};

// Queue handles
QueueHandle_t command_queue = NULL;
QueueHandle_t uart_write_queue = NULL;

// Task handles
TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;
TaskHandle_t xTaskHandle3 = NULL;
TaskHandle_t xTaskHandle4 = NULL;

// Software timer handle
TimerHandle_t led_timer_handle = NULL;

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

  sprintf(user_msg,"\r\nThis is Queue Command Processing demo\n\r");
  printmsg(user_msg);

  // create command queue
  command_queue = xQueueCreate(10,sizeof(APP_CMD_t*));

  // create write queue
  uart_write_queue = xQueueCreate(10,sizeof(char*));


  if((command_queue != NULL) && (uart_write_queue != NULL))
  {
	  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	// create tasks
	xTaskCreate(vTask1_menu_display, "TASK1-MENU" , 500, NULL, 1, &xTaskHandle1);

	xTaskCreate(vTask2_cmd_handling, "TASK2-CMD-HANDLING" , 500, NULL, 2, &xTaskHandle2);

	xTaskCreate(vTask3_cmd_processing, "TASK3-CMD-PROCESS" , 500, NULL, 2, &xTaskHandle3);

	xTaskCreate(vTask4_uart_write, "TASK4-UART-WRITE" , 500, NULL, 2, &xTaskHandle4);

	vTaskStartScheduler();
  }
  else
  {
	sprintf(user_msg,"Queue creation failed\n\r");
	printmsg(user_msg);
  }

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void vTask1_menu_display(void *params)
{
	char *pData = menu;
	while(1)
	{
		xQueueSend(uart_write_queue,&pData,portMAX_DELAY);

		// wait here until someone notifies
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
	}
}

void vTask2_cmd_handling(void *params)
{
	uint8_t command_code = 0;
	APP_CMD_t *new_cmd;

	while(1)
	{
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		new_cmd = (APP_CMD_t*)pvPortMalloc(sizeof(APP_CMD_t));

		taskENTER_CRITICAL();
		command_code = getCommandCode(command_buffer);
		new_cmd->COMMAND_NUM = command_code;
		getArguments(new_cmd->COMMAND_ARGS);
		command_len = 0;
		taskEXIT_CRITICAL();

		// send the command to the command queue
		xQueueSend(command_queue,&new_cmd,portMAX_DELAY);
	}
}

void vTask3_cmd_processing(void *params)
{
	APP_CMD_t *new_cmd;
	// Static ensures the memory persists after this function context changes
	static char task_msg[100];
	char *pMsg = task_msg;

	uint8_t toggle_duration = pdMS_TO_TICKS(500);

	while(1)
	{
		xQueueReceive(command_queue, (void*)&new_cmd, portMAX_DELAY);

		if(new_cmd->COMMAND_NUM == LED_ON_COMMAND)
		{
			make_led_on();
		}
		else if(new_cmd->COMMAND_NUM == LED_OFF_COMMAND)
		{
			make_led_off();
		}
		else if(new_cmd->COMMAND_NUM == LED_TOGGLE_COMMAND)
		{
			led_toggle_start(toggle_duration);
		}
		else if(new_cmd->COMMAND_NUM == LED_TOGGLE_STOP)
		{
			led_toggle_stop();
		}
		else if(new_cmd->COMMAND_NUM == LED_READ_STATUS)
		{
			read_led_status();
			xQueueSend(uart_write_queue, &pMsg, portMAX_DELAY);
		}
		else
		{
			print_error_message(task_msg);
			xQueueSend(uart_write_queue, &pMsg, portMAX_DELAY);
		}

		vPortFree(new_cmd);
	}
}

void vTask4_uart_write(void *params)
{
	char *pData = NULL;
	while(1)
	{
		xQueueReceive(uart_write_queue, &pData, portMAX_DELAY);
		printmsg(pData);
	}
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(huart->Instance == USART2)
	{
		command_buffer[command_len++] = rx_byte;

		if(rx_byte == '\r')
		{
			command_len = 0;

			xTaskNotifyFromISR(xTaskHandle2, 0, eNoAction, &xHigherPriorityTaskWoken);
			xTaskNotifyFromISR(xTaskHandle1, 0, eNoAction, &xHigherPriorityTaskWoken);
		}

		// restart interrupt reception
		HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

uint8_t getCommandCode(uint8_t *buffer)
{
	return buffer[0] - 48;
}

void getArguments(uint8_t *buffer)
{

}

void make_led_on(void)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, SET);
}

void make_led_off(void)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, RESET);
}

void led_toggle(TimerHandle_t xIimer)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
}

void led_toggle_start(uint32_t duration)
{
		if(led_timer_handle == NULL)
		{
			// create timer
			led_timer_handle = xTimerCreate("LED-TIMER",duration,pdTRUE,NULL,led_toggle);

			if(led_timer_handle != NULL)
			{
				xTimerStart(led_timer_handle, 0);
			}
		}
}

void led_toggle_stop(void)
{
	xTimerStop(led_timer_handle,portMAX_DELAY);
}

void read_led_status(void)
{
	char *buf = (char*)pvPortMalloc(32);
	if (buf)
	{
		sprintf(buf, "\r\nLED status: %d\r\n",
				HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_12));
		xQueueSend(uart_write_queue, &buf, portMAX_DELAY);
	}
}


void print_error_message(char* task_msg)
{
	sprintf(task_msg,"\r\nInvalid command received\n\r");
	printmsg(task_msg);
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

    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);

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
