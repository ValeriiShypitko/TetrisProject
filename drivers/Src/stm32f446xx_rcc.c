/*
 * stm32f446xx_rcc.c
 *
 * RCC clock enable/reset lookup tables
 *
 *  Created on: Jul 13, 2026
 *      Author: polly33
 */
#include "stm32f446xx.h"

const RCC_Map_t rcc_en_map[] = {
    [RCC_GPIOA] = {&RCC->AHB1ENR, 0},   [RCC_GPIOB] = {&RCC->AHB1ENR, 1},
    [RCC_GPIOC] = {&RCC->AHB1ENR, 2},   [RCC_GPIOD] = {&RCC->AHB1ENR, 3},
    [RCC_GPIOE] = {&RCC->AHB1ENR, 4},   [RCC_GPIOF] = {&RCC->AHB1ENR, 5},
    [RCC_GPIOG] = {&RCC->AHB1ENR, 6},   [RCC_GPIOH] = {&RCC->AHB1ENR, 7},

    [RCC_USART1] = {&RCC->APB2ENR, 4},  [RCC_USART6] = {&RCC->APB2ENR, 5},
    [RCC_SPI1] = {&RCC->APB2ENR, 12},   [RCC_SPI4] = {&RCC->APB2ENR, 13},
    [RCC_SYSCFG] = {&RCC->APB2ENR, 14},

    [RCC_USART2] = {&RCC->APB1ENR, 17}, [RCC_USART3] = {&RCC->APB1ENR, 18},
    [RCC_UART4] = {&RCC->APB1ENR, 19},  [RCC_UART5] = {&RCC->APB1ENR, 20},
    [RCC_SPI2] = {&RCC->APB1ENR, 14},   [RCC_SPI3] = {&RCC->APB1ENR, 15},
    [RCC_I2C1] = {&RCC->APB1ENR, 21},   [RCC_I2C2] = {&RCC->APB1ENR, 22},
    [RCC_I2C3] = {&RCC->APB1ENR, 23}};

const RCC_Map_t rcc_reset_map[] = {
    [RCC_GPIOA] = {&RCC->AHB1RSTR, 0},   [RCC_GPIOB] = {&RCC->AHB1RSTR, 1},
    [RCC_GPIOC] = {&RCC->AHB1RSTR, 2},   [RCC_GPIOD] = {&RCC->AHB1RSTR, 3},
    [RCC_GPIOE] = {&RCC->AHB1RSTR, 4},   [RCC_GPIOF] = {&RCC->AHB1RSTR, 5},
    [RCC_GPIOG] = {&RCC->AHB1RSTR, 6},   [RCC_GPIOH] = {&RCC->AHB1RSTR, 7},

    [RCC_USART1] = {&RCC->APB2RSTR, 4},  [RCC_USART6] = {&RCC->APB2RSTR, 5},
    [RCC_SPI1] = {&RCC->APB2RSTR, 12},   [RCC_SPI4] = {&RCC->APB2RSTR, 13},
    [RCC_SYSCFG] = {&RCC->APB2RSTR, 14},

    [RCC_USART2] = {&RCC->APB1RSTR, 17}, [RCC_USART3] = {&RCC->APB1RSTR, 18},
    [RCC_UART4] = {&RCC->APB1RSTR, 19},  [RCC_UART5] = {&RCC->APB1RSTR, 20},
    [RCC_SPI2] = {&RCC->APB1RSTR, 14},   [RCC_SPI3] = {&RCC->APB1RSTR, 15},
    [RCC_I2C1] = {&RCC->APB1RSTR, 21},   [RCC_I2C2] = {&RCC->APB1RSTR, 22},
    [RCC_I2C3] = {&RCC->APB1RSTR, 23}};
