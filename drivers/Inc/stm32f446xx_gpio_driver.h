/*
 * stm32f446xx_gpio_driver.h
 *
 *  Created on: May 10, 2026
 *      Author: 2k18h
 */

#ifndef IRC_STM32F446XX_GPIO_DRIVER_H_
#define IRC_STM32F446XX_GPIO_DRIVER_H_
#include "stm32f446xx.h"
/*
 This structure holds the configuration values for GPIO pin
*/
typedef struct {
  uint8_t GPIOx_PinNumber; //	Possible Values from @GPIO_Pin_Macros
  uint8_t GPIOx_PinMode;   //  Possible Values from @@GPIO_Mode_Macros
  uint8_t GPIOx_PinSpeed;  //  Possible Values from @GPIO_OUTPUT_SPEED_Macros
  uint8_t GPIOx_PinOutputType; //  Possible Values from @GPIO_Output_Type_Macros
  uint8_t GPIOx_PinPuPdControl; //  Possible Values from @GPIO_PUPDR_Macros
  uint8_t GPIOx_PinAltFuncMode; //  Possible Values from ?
} GPIO_PinConfig_t;

/*
 This is a Handle Structure for GPIO pin
*/
typedef struct {
  GPIOx_RegDef_t
      *GPIOx; // Holds the base Address of the Port to which the pin belongs
  GPIO_PinConfig_t PinConfig; // Holds the GPIO pin settings
} GPIO_Handle_t;

typedef enum {
  GPIO_PORT_A = 0,
  GPIO_PORT_B,
  GPIO_PORT_C,
  GPIO_PORT_D,
  GPIO_PORT_E,
  GPIO_PORT_F,
  GPIO_PORT_G,
  GPIO_PORT_H
} GPIO_Port_e;

/*=======================
        API prototypes
 ========================*/

/*=======================
        GPIOx Init-DeInit
 ========================*/

void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_OutputInit(GPIOx_RegDef_t *port, uint8_t pinNumber,
                     uint8_t outputSpeed);
void GPIO_DeInit(GPIO_Port_e GPIOPort);

/*=======================
        GPIOx Clock Setup
 ========================*/
void GPIO_PeriClockControl(GPIO_Port_e GPIOPort, uint8_t EnOrDi);

/*==========================
        GPIOx Read-Write data
 ===========================*/

uint8_t GPIO_ReadFromInputPin(GPIOx_RegDef_t *GPIOx, uint8_t pinNumber);
uint16_t GPIO_ReadFromInputPort(GPIOx_RegDef_t *GPIOx);
void GPIO_WriteToOutputPin(GPIOx_RegDef_t *GPIOx, uint8_t pinNumber,
                           uint8_t value);
void GPIO_WriteToOutputPort(GPIOx_RegDef_t *GPIOx, uint16_t value);
void GPIO_ToggleOutputPin(GPIOx_RegDef_t *GPIOx, uint8_t pinNumber);

/*============================================
         GPIOx IRQ configuration and ISR handling
 =============================================*/

void GPIO_IRQHandling(uint8_t pinNumber);
void GPIO_IRQPortForPinSelect(
    uint8_t pin,
    uint8_t
        port_opcode); // Values from @GPIO_Pin_Macros & @GPIOPortForEXTIUpcodes

/* GPIO Macros */
/*
         @GPIO_Mode_Macros
*/
#define GPIO_MODE_IN 0x0U
#define GPIO_MODE_OUT 0x1U
#define GPIO_MODE_ALTFN 0x2U
#define GPIO_MODE_ANALOG 0x3U
#define GPIO_MODE_IT_FT 0x4U
#define GPIO_MODE_IT_RT 0x5U
#define GPIO_MODE_IT_RFT 0x6U

/*
         @GPIO_Output_Type_Macros
*/
#define GPIO_OT_PP 0x0U
#define GPIO_OT_OD 0x1U

/*
         @GPIO_OUTPUT_SPEED_Macros
*/
#define GPIO_OS_LS 0x0U
#define GPIO_OS_MS 0x1U
#define GPIO_OS_FS 0x2U
#define GPIO_OS_HS 0x3U

/*
         @GPIO_PUPDR_Macros
*/
#define GPIO_NOPUD 0x0U
#define GPIO_PU 0x1U
#define GPIO_PD 0x2U

/*
         @GPIO_Pin_Macros
*/

#define GPIO_PIN_0 0x00U
#define GPIO_PIN_1 0x01U
#define GPIO_PIN_2 0x02U
#define GPIO_PIN_3 0x03U
#define GPIO_PIN_4 0x04U
#define GPIO_PIN_5 0x05U
#define GPIO_PIN_6 0x06U
#define GPIO_PIN_7 0x07U
#define GPIO_PIN_8 0x08U
#define GPIO_PIN_9 0x09U
#define GPIO_PIN_10 0xAU
#define GPIO_PIN_11 0xBU
#define GPIO_PIN_12 0xCU
#define GPIO_PIN_13 0xDU
#define GPIO_PIN_14 0xEU
#define GPIO_PIN_15 0xFU

/*
          @GPIOPortForEXTIUpcodes
*/
#define GPIOA_EXTI_OPCODE 0x0
#define GPIOB_EXTI_OPCODE 0x1
#define GPIOC_EXTI_OPCODE 0x2
#define GPIOD_EXTI_OPCODE 0x3
#define GPIOE_EXTI_OPCODE 0x4
#define GPIOF_EXTI_OPCODE 0x5
#define GPIOG_EXTI_OPCODE 0x6
#define GPIOH_EXTI_OPCODE 0x7

#endif /* IRC_STM32F446XX_GPIO_DRIVER_H_ */
