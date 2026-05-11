#include "main.h"
#include "stm32f4xx_hal_dac.h"

DAC_HandleTypeDef hdac;

/* Function Prototypes */
static void MX_GPIO_Init(void);
static void MX_DAC_Init(void);

int main(void)
{
    /* HAL Initialization */
    HAL_Init();

    /* Initialize GPIO */
    MX_GPIO_Init();

    /* Initialize DAC */
    MX_DAC_Init();

    /* Start DAC Channel 1 */
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

    while (1)
    {
        /* Set DAC Value */
        HAL_DAC_SetValue(&hdac,
                         DAC_CHANNEL_1,
                         DAC_ALIGN_12B_R,
                         2048);

        HAL_Delay(1000);
    }
}

/* DAC Initialization */
static void MX_DAC_Init(void)
{
    DAC_ChannelConfTypeDef sConfig = {0};

    /* Enable DAC Clock */
    __HAL_RCC_DAC_CLK_ENABLE();

    /* DAC Instance */
    hdac.Instance = DAC;

    /* Initialize DAC */
    HAL_DAC_Init(&hdac);

    /* DAC Configuration */
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;

    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    HAL_DAC_ConfigChannel(&hdac,
                          &sConfig,
                          DAC_CHANNEL_1);
}

/* GPIO Initialization */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOA Clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA4 as Analog */
    GPIO_InitStruct.Pin  = GPIO_PIN_4;

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;

    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
