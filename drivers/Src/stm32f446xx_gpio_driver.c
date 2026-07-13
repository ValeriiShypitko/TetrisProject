/*
 * stm32f446xx_gpio_driver.c
 *
 *  Created on: May 10, 2026
 *      Author: 2k18h
 */
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx.h"
#include <string.h>

/*=======================
    GPIOx Init-DeInit
 =======================*/

/**
 * @fn              - GPIO_Init
 *
 * @brief           - Initializes the GPIO pin according to the configuration
 *                    specified in GPIO_Handle_t structure
 *
 * @param[in]       - pointer to GPIO handle structure
 *
 * @return          - none
 *
 * @Note            - none
 */

void GPIO_Init(GPIO_Handle_t *pGPIOHandle) {
  uint8_t pinNumber = pGPIOHandle->PinConfig.GPIOx_PinNumber;
  uint8_t mode = pGPIOHandle->PinConfig.GPIOx_PinMode;

  // 1. Configuring Pin Mode
  if (mode <= GPIO_MODE_ANALOG) {
    // Standard GPIO Modes (Input, Output, Alternate Function, Analog)
    pGPIOHandle->GPIOx->MODER &= ~(0x3U << (2 * pinNumber)); // Clear bits
    pGPIOHandle->GPIOx->MODER |= (mode << (2 * pinNumber));  // Set mode
  } else {
    // EXTI Interrupt Modes

    // interrupt modes need the pin as input (MODER = 00)
    pGPIOHandle->GPIOx->MODER &= ~(0x3U << (2 * pinNumber));

    if (mode == GPIO_MODE_IT_FT) {
      // Falling edge trigger activation
      EXTI->FTSR |= (1 << pinNumber);
      // Clear rising edge trigger
      EXTI->RTSR &= ~(1 << pinNumber);
    } else if (mode == GPIO_MODE_IT_RT) {
      // Rising edge trigger activation
      EXTI->RTSR |= (1 << pinNumber);
      // Clear falling edge trigger
      EXTI->FTSR &= ~(1 << pinNumber);
    } else if (mode == GPIO_MODE_IT_RFT) {
      // Falling and rising edge activation
      EXTI->RTSR |= (1 << pinNumber);
      EXTI->FTSR |= (1 << pinNumber);
    }

    // 2. Configuring the GPIO selection in SYSCFG_EXTICR
    RCC_Enable_Clock(RCC_SYSCFG);

    uint8_t extiRegIndex = pinNumber / 4;
    uint8_t extiBitPos = (pinNumber % 4) * 4;
    uint8_t portCode = GPIOx_BaseAddr_To_Code(pGPIOHandle->GPIOx);

    // clear, then set the port code
    SYSCFG->EXTICR[extiRegIndex] &= ~(0xFU << extiBitPos);
    SYSCFG->EXTICR[extiRegIndex] |= (portCode << extiBitPos);

    // 3. Enabling the EXTI interrupt delivery using IMR
    EXTI->IMR |= (1 << pinNumber);
  }

  // 4. Configuring Pin Speed
  pGPIOHandle->GPIOx->OSPEEDR &= ~(0x3U << (2 * pinNumber)); // Clear bits
  pGPIOHandle->GPIOx->OSPEEDR |=
      (pGPIOHandle->PinConfig.GPIOx_PinSpeed << (2 * pinNumber));

  // 5. Configuring PUPD (Pull-Up/Pull-Down) settings
  pGPIOHandle->GPIOx->PUPDR &= ~(0x3U << (2 * pinNumber)); // Clear bits
  pGPIOHandle->GPIOx->PUPDR |=
      (pGPIOHandle->PinConfig.GPIOx_PinPuPdControl << (2 * pinNumber));

  // 6. Configuring The Output Type (Push-Pull or Open-Drain)
  pGPIOHandle->GPIOx->OTYPER &= ~(0x1U << pinNumber); // Clear bit
  pGPIOHandle->GPIOx->OTYPER |=
      (pGPIOHandle->PinConfig.GPIOx_PinOutputType << pinNumber);

  // 7. Configuring Alternate Functionality
  if (mode == GPIO_MODE_ALTFN) {
    if (pinNumber < 8) {
      // Low Register (AFRL) handles pins 0 to 7
      pGPIOHandle->GPIOx->AFRL &= ~(0xFU << (4 * pinNumber));
      pGPIOHandle->GPIOx->AFRL |=
          (pGPIOHandle->PinConfig.GPIOx_PinAltFuncMode << (4 * pinNumber));
    } else {
      // High Register (AFRH) handles pins 8 to 15
      pGPIOHandle->GPIOx->AFRH &= ~(0xFU << (4 * (pinNumber - 8)));
      pGPIOHandle->GPIOx->AFRH |= (pGPIOHandle->PinConfig.GPIOx_PinAltFuncMode
                                   << (4 * (pinNumber - 8)));
    }
  }
}
/**
 * @fn              - GPIO_OutputInit
 *
 * @brief           - Initializes the GPIO pin according to the configuration
 *                    specified in GPIO_Handle_t structure
 *
 * @param[in]       - GPIOx_RegDef_t *port - pointer to GPIO RegDef structure
 * Peripheral definitions Possible Values from @Peripheral definitions
 * @param[in]       - uint8_t pinNumber    - pin number.
 * Possible Values from @GPIO_Pin_Macros
 * @param[in]       - uint8_t outputSpeed  - output speed parameter. Possible
 * Values from @GPIO_OUTPUT_SPEED_Macros
 * @return          - none
 *
 * @Note            - none
 */

void GPIO_OutputInit(GPIOx_RegDef_t *port, uint8_t pinNumber,
                     uint8_t outputSpeed) {
  GPIO_Handle_t gpioHandler;
  memset(&gpioHandler, 0, sizeof(gpioHandler));

  gpioHandler.GPIOx = port;
  gpioHandler.PinConfig.GPIOx_PinMode = GPIO_MODE_OUT;
  gpioHandler.PinConfig.GPIOx_PinNumber = pinNumber;
  gpioHandler.PinConfig.GPIOx_PinSpeed = outputSpeed;
  gpioHandler.PinConfig.GPIOx_PinPuPdControl = GPIO_NOPUD;
  gpioHandler.PinConfig.GPIOx_PinOutputType = GPIO_OT_PP;
  GPIO_Init(&gpioHandler);
}

/**
 * @fn              - GPIO_DeInit
 *
 * @brief           - Resets the selected GPIO peripheral registers
 *                    to their default reset values
 *
 * @param[in]       - base address of the GPIO peripheral
 *
 * @return          - none
 *
 * @Note            - none
 */
void GPIO_DeInit(GPIO_Port_e GPIOPort) {
  *rcc_reset_map[GPIOPort].reg |=  (0x1U << rcc_reset_map[GPIOPort].bit);
  *rcc_reset_map[GPIOPort].reg &= ~(0x1U << rcc_reset_map[GPIOPort].bit);
}

/*=======================
    GPIOx Clock Setup
 =======================*/

/**
 * @fn              - GPIO_PeriClockControl
 *
 * @brief           - Enables or disables peripheral clock
 *                    for the given GPIO port
 *
 * @param[in]       - base address of GPIO peripheral
 * @param[in]       - ENABLE or DISABLE macros
 *
 * @return          - none
 *
 * @Note            - none
 */
void GPIO_PeriClockControl(GPIO_Port_e GPIOPort, uint8_t EnOrDi) {
  if (EnOrDi == ENABLE) {
    RCC_Enable_Clock((RCC_Periph_e)GPIOPort);
  } else {
    RCC_Disable_Clock((RCC_Periph_e)GPIOPort);
  }
}

/*==========================
    GPIOx Read-Write data
 ===========================*/

/**
 * @fn              - GPIO_ReadFromInputPin
 *
 * @brief           - Reads the value from a given GPIO input pin
 *
 * @param[in]       - base address of GPIO peripheral
 * @param[in]       - GPIO pin number
 *
 * @return          - uint8_t value
 *
 * @Note            - none
 */
uint8_t GPIO_ReadFromInputPin(GPIOx_RegDef_t *GPIOx, uint8_t pinNumber) {
  return (uint8_t)((GPIOx->IDR >> pinNumber) & 0x1U);
}

/**
 * @fn              - GPIO_ReadFromInputPort
 *
 * @brief           - Reads the value from the entire GPIO input port
 *
 * @param[in]       - base address of GPIO peripheral
 *
 * @return          - uint16_t value
 *
 * @Note            - none
 */
uint16_t GPIO_ReadFromInputPort(GPIOx_RegDef_t *GPIOx) {
  return (uint16_t)(GPIOx->IDR);
}

/**
 * @fn              - GPIO_WriteToOutputPin
 *
 * @brief           - Writes a value to the specified GPIO output pin
 *
 * @param[in]       - base address of GPIO peripheral
 * @param[in]       - GPIO pin number
 * @param[in]       - value to be written
 *
 * @return          - none
 *
 * @Note            - none
 */
void GPIO_WriteToOutputPin(GPIOx_RegDef_t *GPIOx, uint8_t pinNumber,
                           uint8_t value) {
  if (value == GPIO_PIN_SET) {
    GPIOx->ODR |= (0x1U << pinNumber);
  } else {
    GPIOx->ODR &= ~(0x1U << pinNumber);
  }
}

/**
 * @fn              - GPIO_WriteToOutputPort
 *
 * @brief           - Writes a value to the entire GPIO output port
 *
 * @param[in]       - base address of GPIO peripheral
 * @param[in]       - value to be written
 *
 * @return          - none
 *
 * @Note            - none
 */
void GPIO_WriteToOutputPort(GPIOx_RegDef_t *GPIOx, uint16_t value) {
  GPIOx->ODR = value;
}

/**
 * @fn              - GPIO_ToggleOutputPin
 *
 * @brief           - Toggles the specified GPIO output pin
 *
 * @param[in]       - base address of GPIO peripheral
 * @param[in]       - GPIO pin number
 *
 * @return          - none
 *
 * @Note            - none
 */
void GPIO_ToggleOutputPin(GPIOx_RegDef_t *GPIOx, uint8_t pinNumber) {
  GPIOx->ODR ^= (1 << pinNumber);
}

/*============================================
     GPIOx ISR handling
 ============================================*/

/**
 * @fn              - GPIO_IRQHandling
 *
 * @brief           - Handles GPIO interrupt for the specified pin
 *
 * @param[in]       - GPIO pin number
 *
 * @return          - none
 *
 * @Note            - none
 */

void GPIO_IRQHandling(uint8_t pinNumber) {
  if (pinNumber > 15)
    return;
  // PR is write-1-to-clear, never read-modify-write it
  EXTI->PR = (0x1U << pinNumber);
}

/**
 * @fn              -GPIO_IRQHandling
 *
 * @brief           -Selects the port for the EXTI
 *
 * @param[in]       - GPIO pin number
 * @param[in]       - GPIO port opcode
 * @return          - none
 *
 * @Note            - none
 */

void GPIO_IRQPortForPinSelect(uint8_t pin, uint8_t port_opcode) {
  if (pin > 15)
    return;
  uint8_t tempBitPos = pin % 4;
  uint8_t tempRegPos = pin / 4;
  SYSCFG->EXTICR[tempRegPos] &= ~(0xFU << (4 * tempBitPos));
  SYSCFG->EXTICR[tempRegPos] |= (port_opcode << (4 * tempBitPos));
}
