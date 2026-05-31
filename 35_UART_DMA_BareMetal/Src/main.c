#include "stm32f407xx.h"
#include <string.h>

/* ---------------- DMA REGISTER DEFINITIONS ---------------- */

#define DMA1_BASE      0x40026000U

#define DMA1_HISR      (*(volatile uint32_t *)(DMA1_BASE + 0x04))
#define DMA1_HIFCR     (*(volatile uint32_t *)(DMA1_BASE + 0x0C))

/* Stream5 RX */
#define DMA1_S5CR      (*(volatile uint32_t *)(DMA1_BASE + 0x88))
#define DMA1_S5NDTR    (*(volatile uint32_t *)(DMA1_BASE + 0x8C))
#define DMA1_S5PAR     (*(volatile uint32_t *)(DMA1_BASE + 0x90))
#define DMA1_S5M0AR    (*(volatile uint32_t *)(DMA1_BASE + 0x94))

/* Stream6 TX */
#define DMA1_S6CR      (*(volatile uint32_t *)(DMA1_BASE + 0xA0))
#define DMA1_S6NDTR    (*(volatile uint32_t *)(DMA1_BASE + 0xA4))
#define DMA1_S6PAR     (*(volatile uint32_t *)(DMA1_BASE + 0xA8))
#define DMA1_S6M0AR    (*(volatile uint32_t *)(DMA1_BASE + 0xAC))

/* ---------------- Buffers ---------------- */

uint8_t tx_data[] = "Hello DMA UART\r\n";
uint8_t rx_buffer[50];

/* ---------------- Function Prototypes ---------------- */

void USART2_DMA_Init(void);
void USART2_DMA_TX(uint8_t *buffer, uint32_t len);
void USART2_DMA_RX(uint8_t *buffer, uint32_t len);

/* ---------------- MAIN ---------------- */

int main(void)
{
    USART2_DMA_Init();

    while(1)
    {
        /* Start RX DMA */
        USART2_DMA_RX(rx_buffer, sizeof(rx_buffer));

        /* Start TX DMA */
        USART2_DMA_TX(tx_data, strlen((char*)tx_data));

        for(volatile int i = 0; i < 3000000; i++);
    }
}

/* ---------------- USART2 INIT ---------------- */

void USART2_DMA_Init(void)
{
    /* Enable Clocks */
    RCC->AHB1ENR |= (1 << 0);      // GPIOA Clock
    RCC->AHB1ENR |= (1 << 21);     // DMA1 Clock
    RCC->APB1ENR |= (1 << 17);     // USART2 Clock

    /* PA2 and PA3 Alternate Function Mode */

    /* MODER */
    (*(volatile uint32_t *)(GPIOA + 0x00)) &= ~(0xF << 4);
    (*(volatile uint32_t *)(GPIOA + 0x00)) |=  (0xA << 4);

    /* AFRL -> AF7 */
    (*(volatile uint32_t *)(GPIOA + 0x20)) &= ~(0xFF << 8);
    (*(volatile uint32_t *)(GPIOA + 0x20)) |=  (0x77 << 8);

    /* Baudrate = 9600 */
    USART2->BRR = 0x0683;

    /* Enable DMA TX and RX */
    USART2->CR3 |= (1 << 7);   // DMAT
    USART2->CR3 |= (1 << 6);   // DMAR

    /* Enable TX and RX */
    USART2->CR1 |= (1 << 3);   // TE
    USART2->CR1 |= (1 << 2);   // RE

    /* Enable USART */
    USART2->CR1 |= (1 << 13);  // UE
}

/* ---------------- DMA TX ---------------- */

void USART2_DMA_TX(uint8_t *buffer, uint32_t len)
{
    /* Disable Stream6 */
    DMA1_S6CR &= ~(1 << 0);

    while(DMA1_S6CR & (1 << 0));

    /* Clear all Stream6 flags */
    DMA1_HIFCR |= (0x3D << 16);

    /* Peripheral address = USART2 DR */
    DMA1_S6PAR = (uint32_t)&USART2->DR;

    /* Memory address */
    DMA1_S6M0AR = (uint32_t)buffer;

    /* Number of bytes */
    DMA1_S6NDTR = len;

    /* Clear CR */
    DMA1_S6CR = 0;

    /* Channel 4 */
    DMA1_S6CR |= (4 << 25);

    /* Memory increment mode */
    DMA1_S6CR |= (1 << 10);

    /* Memory to Peripheral */
    DMA1_S6CR |= (1 << 6);

    /* Enable DMA Stream */
    DMA1_S6CR |= (1 << 0);

    /* Wait for transfer complete */
    while(!(DMA1_HISR & (1 << 21)));

    /* Clear transfer complete flag */
    DMA1_HIFCR |= (1 << 21);
}

/* ---------------- DMA RX ---------------- */

void USART2_DMA_RX(uint8_t *buffer, uint32_t len)
{
    /* Disable Stream5 */
    DMA1_S5CR &= ~(1 << 0);

    while(DMA1_S5CR & (1 << 0));

    /* Clear all Stream5 flags */
    DMA1_HIFCR |= (0x3D << 6);

    /* Peripheral address = USART2 DR */
    DMA1_S5PAR = (uint32_t)&USART2->DR;

    /* Memory address */
    DMA1_S5M0AR = (uint32_t)buffer;

    /* Number of bytes */
    DMA1_S5NDTR = len;

    /* Clear CR */
    DMA1_S5CR = 0;

    /* Channel 4 */
    DMA1_S5CR |= (4 << 25);

    /* Memory increment mode */
    DMA1_S5CR |= (1 << 10);

    /* Peripheral to Memory */
    DMA1_S5CR &= ~(1 << 6);

    /* Enable DMA Stream */
    DMA1_S5CR |= (1 << 0);
}
