#include "stm32f4xx_hal.h"

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  uint8_t mode = 0;

  while (1)
  {
    // Check button press (PA0)
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
      HAL_Delay(200); // debounce

      mode = !mode; // toggle between 0 and 1
      // Wait until button released
      while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET);

      // Turn OFF all LEDs
      HAL_GPIO_WritePin(GPIOD,
        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
        GPIO_PIN_RESET);

      // Turn ON selected LEDs
      if(mode == 0)
      {
        // First group
        HAL_GPIO_WritePin(GPIOD,
          GPIO_PIN_12 | GPIO_PIN_13,
          GPIO_PIN_SET);
      }
      else
      {
        // Second group
        HAL_GPIO_WritePin(GPIOD,
          GPIO_PIN_14 | GPIO_PIN_15,
          GPIO_PIN_SET);
      }
    }
  }
}

/* GPIO Initialization */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Enable clocks
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  // Configure LED pins PD12–PD15
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // Configure Button PA0
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* System Clock Config */
void SystemClock_Config(void)
{
}
