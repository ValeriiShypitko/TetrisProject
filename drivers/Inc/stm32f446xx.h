#ifndef IRC_STM32F446XX_H_
#define IRC_STM32F446XX_H_
#include <stdint.h>

#define __I volatile const // Read Only Register
#define __O volatile       // Write Only Register
#define __IO volatile      // Read / Write Register
/* =========================
   ARM Cortex M4 specific details
   ========================= */
/* =========================
   ARM Cortex M4 processor NVIC ISERx register addresses
   ========================= */

/* =========================
   ARM Cortex M4 processor RegDef Structs
   ========================= */
typedef struct {
  __IO uint32_t ISER[8]; // Interrupt Set-enable Registers
  uint32_t RESERVED0[24];
  __IO uint32_t ICER[8]; // Interrupt Clear-enable Registers
  uint32_t RESERVED1[24];
  __IO uint32_t ISPR[8]; // Interrupt Set-pending Registers
  uint32_t RESERVED2[24];
  __IO uint32_t ICPR[8]; // Interrupt Clear-pending registers
  uint32_t RESERVED3[24];
  __I uint32_t IABR[8]; // Interrupt Active Bit Registers
  uint32_t RESERVED4[56];
  __IO uint32_t IPR[60]; // Interrupt Priority Registers
  uint32_t RESERVED5[644];
  __O uint32_t STIR; // Software Trigger Interrupt
} NVIC_RegDef_t;

typedef struct {
  __IO uint32_t ACTLR; // Auxiliary Control Register: Controls Cortex-M specific
                       // features like folding, etc.
  __IO uint32_t RESERVED0[829]; // padding up to CPUID
  __I uint32_t CPUID; // CPUID Base Register: Contains processor part number,
                      // version, and implementation details.
  __IO uint32_t ICSR; // Interrupt Control and State Register: Used to set/clear
                      // pending exceptions and read system state.
  __IO uint32_t VTOR; // Vector Table Offset Register: Points to the base
                      // address of the exception/interrupt vector table.
  __IO uint32_t
      AIRCR; // Application Interrupt and Reset Control Register: Used for
             // endianness configuration and triggering a software reset.
  __IO uint32_t SCR;   // System Control Register: Controls sleep behavior and
                       // power management (e.g., Sleep-on-Exit).
  __IO uint32_t CCR;   // Configuration and Control Register: Controls entry to
                       // unaligned memory traps, divide-by-zero traps, etc.
  __IO uint32_t SHPR1; // System Handler Priority Register 1: Sets priority for
                       // MemManage, BusFault, and UsageFault exceptions.
  __IO uint32_t
      SHPR2; // System Handler Priority Register 2: Sets priority for SVCall.
  __IO uint32_t SHPR3; // System Handler Priority Register 3: Sets priority for
                       // SysTick and PendSV.
  __IO uint32_t SHCRS; // System Handler Control and State Register: Enables
                       // system handlers and tracks if they are active/pending.

  union {
    __IO uint32_t CFSR; // Configurable Fault Status Register: A 32-bit register
                        // combining three 8/16-bit fault registers.
    struct {
      __IO uint8_t MMSR;  // MemManage Fault Status Register (Bits 0-7 of CFSR).
      __IO uint8_t BFSR;  // BusFault Status Register (Bits 8-15 of CFSR).
      __IO uint16_t UFSR; // UsageFault Status Register (Bits 16-31 of CFSR).
    };
  };

  __IO uint32_t HFSR; // HardFault Status Register: Gives information about
                      // events that trigger a HardFault.
  __IO uint32_t RESERVED1; // padding (DFSR slot)
  __IO uint32_t MMAR; // MemManage Fault Address Register: Holds the address
                      // that triggered the MemManage fault.
  __IO uint32_t BFAR; // BusFault Address Register: Holds the address that
                      // triggered the BusFault.
  __IO uint32_t AFSR; // Auxiliary Fault Status Register: Contains
                      // vendor-specific fault information.
} SCB_RegDef_t;

typedef struct {
  __IO uint32_t SYST_CSR;  // SysTick Control and Status Register
  __IO uint32_t SYST_RVR;  // SysTick Reload Value Register
  __IO uint32_t SYST_CVR;  // SysTick Current Value Register
  __I uint32_t SYST_CALIB; // SysTick Calibration Value Register
} SysTick_RegDef_t;

// ARM Cortex M4 Base Addresses
#define NVIC_BASEADDR (0xE000E100UL)
#define SCB_BASEADDR (0xE000E008UL)
#define SYSTICK_BASEADDR (0xE000E010UL)

/* =========================
   Peripheral definitions (Peripheral base addresses typecasted to xx_TypeDef_t)
   ========================= */

#define NVIC                                                                   \
  ((NVIC_RegDef_t *)NVIC_BASEADDR)         // Nested Vector Interrupt Controller
#define SCB ((SCB_RegDef_t *)SCB_BASEADDR) // System Control Block
#define SYSTICK ((SysTick_RegDef_t *)SYSTICK_BASEADDR) // System Timer SysTick

/* =========================
   SysTick Control an Status Register macros
   ========================= */
#define SYSTICK_ENABLE (1U << 0)    // Indicates the clock source:
#define SYSTICK_TICKINT (1U << 1)   // Enables SysTick exception request
#define SYSTICK_CLKSOURCE (1U << 2) // Indicates the clock source
#define SYSTICK_COUNTFLAG                                                      \
  (1U << 16) // Returns 1 if timer counted to 0 since last time this was read.

/* =========================
   SCB ICSR bit macros
   ========================= */
#define SCB_ICSR_PENDSVSET (1U << 28) // Pends the PendSV exception

/* =========================
   STM32F446RE(MCU) specific details
   ========================= */

// Clock Speed
#define CLOCKSPEED 16000000UL

/* =========================
   Memory regions
   ========================= */

#define FLASH_BASEADDR 0x08000000U
#define SRAM_BASEADDR 0x20000000U
#define SRAM1_BASEADDR SRAM_BASEADDR
#define SRAM2_BASEADDR 0x2001C000U
#define SYSTEM_MEMORY_BASEADDR 0x1FFF0000U

/* =========================
   Structures
   ========================= */

typedef struct {
  __IO uint32_t MODER;   // GPIO port mode register
  __IO uint32_t OTYPER;  // GPIO port output type register
  __IO uint32_t OSPEEDR; // GPIO port output speed register
  __IO uint32_t PUPDR;   // GPIO port pull-up/pull-down register
  __I uint32_t IDR;      // GPIO port input data register
  __IO uint32_t ODR;     // GPIO port output data register
  __IO uint32_t BSRR;    // GPIO port bit set/reset register
  __IO uint32_t LCKR;    // GPIO port configuration lock register
  __IO uint32_t AFRL;    // GPIO alternate function low register
  __IO uint32_t AFRH;    // GPIO alternate function high register
} GPIOx_RegDef_t;

typedef struct {
  __IO uint32_t CR;
  __IO uint32_t PLLCFGR;
  __IO uint32_t CFGR;
  __IO uint32_t CIR;
  __IO uint32_t AHB1RSTR;
  __IO uint32_t AHB2RSTR;
  __IO uint32_t AHB3RSTR;
  __I uint32_t RESERVED1;
  __IO uint32_t APB1RSTR;
  __IO uint32_t APB2RSTR;
  __I uint32_t RESERVED2[2];
  __IO uint32_t AHB1ENR;
  __IO uint32_t AHB2ENR;
  __IO uint32_t AHB3ENR;
  __I uint32_t RESERVED3;
  __IO uint32_t APB1ENR;
  __IO uint32_t APB2ENR;
  __I uint32_t RESERVED4[2];
  __IO uint32_t AHB1LPENR;
  __IO uint32_t AHB2LPENR;
  __IO uint32_t AHB3LPENR;
  __I uint32_t RESERVED5;
  __IO uint32_t APB1LPENR;
  __IO uint32_t APB2LPENR;
  __I uint32_t RESERVED6[2];
  __IO uint32_t BDCR;
  __IO uint32_t CSR;
  __I uint32_t RESERVED7[2];
  __IO uint32_t SSCGR;
  __IO uint32_t PLLI2SCFGR;
  __IO uint32_t PLLSAICFGR;
  __IO uint32_t DCKCFGR;
  __IO uint32_t CKGATENR;
  __IO uint32_t DCKCFGR2;
} RCC_RegDef_t;

typedef struct {     // Only bits from [0,22] can be used, [23..31] are reserved
  __IO uint32_t IMR; // Interrupt mask register
  __IO uint32_t EMR; // Event mask register
  // For R/F edge triggers also 19 bit is reserved
  __IO uint32_t RTSR;  // Rising trigger selection register
  __IO uint32_t FTSR;  // Falling trigger selection register
  __IO uint32_t SWIER; // Software interrupt event register
  __IO uint32_t PR;    // Pending register
} EXTI_RegDef_t;

typedef struct {
  __IO uint32_t MEMRMP; // SYSCFG Memory remap register
  __IO uint32_t PMC;    // SYSCFG peripheral mode configuration register
  __IO uint32_t
      EXTICR[4]; // SYSCFG external interrupt configuration register [1..4]
  uint32_t RESERVED1[2]; // Reserved
  __IO uint32_t CMPCR;   // Compensation cell control register register
  uint32_t RESERVED2[2]; // Reserved
  __IO uint32_t CFGR;    // SYSCFG configuration register

} SYSCFG_RegDef_t;

typedef struct {
  __IO uint16_t SPI_CR1; // SPI control register 1 (not used in I2S mode)
  uint16_t RESERVED1;

  __IO uint16_t SPI_CR2; // SPI control register 2
  uint16_t RESERVED2;

  __IO uint16_t SPI_SR; // SPI status register
  uint16_t RESERVED3;

  __IO uint16_t SPI_DR; // SPI data register
  uint16_t RESERVED4;

  __IO uint16_t SPI_CRCPR; // SPI CRC polynomial register (not used in I2S mode)
  uint16_t RESERVED5;

  __I uint16_t SPI_RXCRCR; // SPI RX CRC register (not used in I2S mode)
  uint16_t RESERVED6;

  __I uint16_t SPI_TXCRCR; // SPI TX CRC register (not used in I2S mode)
  uint16_t RESERVED7;

  __IO uint16_t SPI_I2SCFGR; // SPI_I2S configuration register
  uint16_t RESERVED8;

  __IO uint16_t SPI_I2SPR; // SPI_I2S prescaler register
  uint16_t RESERVED9;

} SPI_RegDef_t;

typedef struct {
  __IO uint32_t SR;   // USART status register
  __IO uint32_t DR;   // USART data register
  __IO uint32_t BRR;  // USART baud rate register
  __IO uint32_t CR1;  // USART control register 1
  __IO uint32_t CR2;  // USART control register 2
  __IO uint32_t CR3;  // USART control register 3
  __IO uint32_t GTPR; // USART guard time and prescaler register
} USART_RegDef_t;

/* =========================
   Bus base addresses
   ========================= */

#define PERIPH_BASE 0x40000000UL

#define APB1_BASEADDR (PERIPH_BASE + 0x00000U)
#define APB2_BASEADDR (PERIPH_BASE + 0x10000U)
#define AHB1_BASEADDR (PERIPH_BASE + 0x20000U)

#define AHB2_BASEADDR 0x50000000UL
#define AHB3_BASEADDR 0xA0000000UL

/* =========================
   AHB1 peripherals
   ========================= */

#define GPIOA_BASEADDR (AHB1_BASEADDR + 0x0000)
#define GPIOB_BASEADDR (AHB1_BASEADDR + 0x0400)
#define GPIOC_BASEADDR (AHB1_BASEADDR + 0x0800)
#define GPIOD_BASEADDR (AHB1_BASEADDR + 0x0C00)
#define GPIOE_BASEADDR (AHB1_BASEADDR + 0x1000)
#define GPIOF_BASEADDR (AHB1_BASEADDR + 0x1400)
#define GPIOG_BASEADDR (AHB1_BASEADDR + 0x1800)
#define GPIOH_BASEADDR (AHB1_BASEADDR + 0x1C00)

#define CRC_BASEADDR (AHB1_BASEADDR + 0x3000)
#define RCC_BASEADDR (AHB1_BASEADDR + 0x3800)
#define FLASH_INT_REG_BASEADDR (AHB1_BASEADDR + 0x3C00)

#define BKPSRAM_BASEADDR (AHB1_BASEADDR + 0x4000)

#define DMA1_BASEADDR (AHB1_BASEADDR + 0x6000)
#define DMA2_BASEADDR (AHB1_BASEADDR + 0x6400)

/* =========================
   APB1 peripherals
   ========================= */

#define TIM2_BASEADDR (APB1_BASEADDR + 0x0000)
#define TIM3_BASEADDR (APB1_BASEADDR + 0x0400)
#define TIM4_BASEADDR (APB1_BASEADDR + 0x0800)
#define TIM5_BASEADDR (APB1_BASEADDR + 0x0C00)

#define TIM6_BASEADDR (APB1_BASEADDR + 0x1000)
#define TIM7_BASEADDR (APB1_BASEADDR + 0x1400)

#define TIM12_BASEADDR (APB1_BASEADDR + 0x1800)
#define TIM13_BASEADDR (APB1_BASEADDR + 0x1C00)
#define TIM14_BASEADDR (APB1_BASEADDR + 0x2000)

#define RTC_BKP_REG_BASEADDR (APB1_BASEADDR + 0x2800)
#define WWDG_BASEADDR (APB1_BASEADDR + 0x2C00)
#define IWDG_BASEADDR (APB1_BASEADDR + 0x3000)

#define SPI2_BASEADDR (APB1_BASEADDR + 0x3800)
#define SPI3_BASEADDR (APB1_BASEADDR + 0x3C00)

#define USART2_BASEADDR (APB1_BASEADDR + 0x4400)
#define USART3_BASEADDR (APB1_BASEADDR + 0x4800)
#define UART4_BASEADDR (APB1_BASEADDR + 0x4C00)
#define UART5_BASEADDR (APB1_BASEADDR + 0x5000)

#define I2C1_BASEADDR (APB1_BASEADDR + 0x5400)
#define I2C2_BASEADDR (APB1_BASEADDR + 0x5800)
#define I2C3_BASEADDR (APB1_BASEADDR + 0x5C00)

#define CAN1_BASEADDR (APB1_BASEADDR + 0x6400)
#define CAN2_BASEADDR (APB1_BASEADDR + 0x6800)

#define PWR_BASEADDR (APB1_BASEADDR + 0x7000)
#define DAC_BASEADDR (APB1_BASEADDR + 0x7400)

/* =========================
   APB2 peripherals
   ========================= */

#define TIM1_BASEADDR (APB2_BASEADDR + 0x0000)
#define TIM8_BASEADDR (APB2_BASEADDR + 0x0400)

#define USART1_BASEADDR (APB2_BASEADDR + 0x1000)
#define USART6_BASEADDR (APB2_BASEADDR + 0x1400)

#define ADC_BASEADDR (APB2_BASEADDR + 0x2000)

#define SDMMC_BASEADDR (APB2_BASEADDR + 0x2C00)

#define SPI1_BASEADDR (APB2_BASEADDR + 0x3000)
#define SPI4_BASEADDR (APB2_BASEADDR + 0x3400)

#define SYSCFG_BASEADDR (APB2_BASEADDR + 0x3800)
#define EXTI_BASEADDR (APB2_BASEADDR + 0x3C00)

#define TIM9_BASEADDR (APB2_BASEADDR + 0x4000)
#define TIM10_BASEADDR (APB2_BASEADDR + 0x4400)
#define TIM11_BASEADDR (APB2_BASEADDR + 0x4800)

#define SAI1_BASEADDR (APB2_BASEADDR + 0x5800)
#define SAI2_BASEADDR (APB2_BASEADDR + 0x5C00)

/* =========================
   AHB2 peripherals
   ========================= */

#define USB_OTG_FS_BASEADDR (AHB2_BASEADDR + 0x00000)
#define DCMI_BASEADDR (AHB2_BASEADDR + 0x50000)

/* =========================
   AHB3 peripherals
   ========================= */

#define FMC_BASEADDR (AHB3_BASEADDR + 0x0000)
#define QUADSPI_BASEADDR (AHB3_BASEADDR + 0x1000)

/* =========================
   IRQ Interrupt Numbers
   ========================= */
#define IRQ_NO_EXTI0 6U
#define IRQ_NO_EXTI1 7U
#define IRQ_NO_EXTI2 8U
#define IRQ_NO_EXTI3 9U
#define IRQ_NO_EXTI4 10U
#define IRQ_NO_EXTI9_5 23U
#define IRQ_NO_EXTI15_10 40U

#define IRQ_NO_SPI1 35U
#define IRQ_NO_SPI2 36U
#define IRQ_NO_SPI3 51U
#define IRQ_NO_SPI4 84U

/* =========================
   NVIC Priority Levels
   ========================= */

#define NVIC_PRI_0 0U
#define NVIC_PRI_1 1U
#define NVIC_PRI_2 2U
#define NVIC_PRI_3 3U
#define NVIC_PRI_4 4U
#define NVIC_PRI_5 5U
#define NVIC_PRI_6 6U
#define NVIC_PRI_7 7U
#define NVIC_PRI_8 8U
#define NVIC_PRI_9 9U
#define NVIC_PRI_10 10U
#define NVIC_PRI_11 11U
#define NVIC_PRI_12 12U
#define NVIC_PRI_13 13U
#define NVIC_PRI_14 14U
#define NVIC_PRI_15 15U

// Priority bits implemented in STM32F446RE
#define NO_PR_BITS_IMPLEMENTED 4U

/* =========================
   Peripheral definitions (Peripheral base addresses typecasted to xx_TypeDef_t)
   ========================= */

#define GPIOA ((GPIOx_RegDef_t *)GPIOA_BASEADDR)
#define GPIOB ((GPIOx_RegDef_t *)GPIOB_BASEADDR)
#define GPIOC ((GPIOx_RegDef_t *)GPIOC_BASEADDR)
#define GPIOD ((GPIOx_RegDef_t *)GPIOD_BASEADDR)
#define GPIOE ((GPIOx_RegDef_t *)GPIOE_BASEADDR)
#define GPIOF ((GPIOx_RegDef_t *)GPIOF_BASEADDR)
#define GPIOG ((GPIOx_RegDef_t *)GPIOG_BASEADDR)
#define GPIOH ((GPIOx_RegDef_t *)GPIOH_BASEADDR)
#define RCC ((RCC_RegDef_t *)RCC_BASEADDR)
#define EXTI ((EXTI_RegDef_t *)EXTI_BASEADDR)
#define SYSCFG ((SYSCFG_RegDef_t *)SYSCFG_BASEADDR)
#define SPI1 ((SPI_RegDef_t *)SPI1_BASEADDR)
#define SPI2 ((SPI_RegDef_t *)SPI2_BASEADDR)
#define SPI3 ((SPI_RegDef_t *)SPI3_BASEADDR)
#define SPI4 ((SPI_RegDef_t *)SPI4_BASEADDR)
#define USART2 ((USART_RegDef_t *)USART2_BASEADDR)

/* =========================
   Clock Enable Structure and enum
   ========================= */
typedef enum {
  RCC_GPIOA = 0,
  RCC_GPIOB,
  RCC_GPIOC,
  RCC_GPIOD,
  RCC_GPIOE,
  RCC_GPIOF,
  RCC_GPIOG,
  RCC_GPIOH,

  RCC_USART1,
  RCC_USART2,
  RCC_USART3,
  RCC_UART4,
  RCC_UART5,
  RCC_USART6,

  RCC_SPI1,
  RCC_SPI2,
  RCC_SPI3,
  RCC_SPI4,

  RCC_I2C1,
  RCC_I2C2,
  RCC_I2C3,

  RCC_SYSCFG
} RCC_Periph_e;

// RCC set/reset map structures
typedef struct {
  __IO uint32_t *reg;
  uint8_t bit;
} RCC_Map_t;

// defined in stm32f446xx_rcc.c
extern const RCC_Map_t rcc_en_map[];
extern const RCC_Map_t rcc_reset_map[];
/* =========================
   Inline Fuctions
   ========================= */

/**
 * @fn              - RCC_Enable_Clock
 *
 * @brief           - Enables RCC Clock for given peripheral
 * @param[in]       - RCC_Periph_e periph
 *
 * @return          - none
 *
 * @Note            - none
 */
static inline void RCC_Enable_Clock(RCC_Periph_e periph) {
  *rcc_en_map[periph].reg |= (1U << rcc_en_map[periph].bit);
}

/**
 * @fn              - RCC_Disable_Clock
 *
 * @brief           - Disables RCC Clock for given peripheral
 * @param[in]       - RCC_Periph_e periph
 *
 * @return          - none
 *
 * @Note            - none
 */
static inline void RCC_Disable_Clock(RCC_Periph_e periph) {
  *rcc_en_map[periph].reg &= ~(1U << rcc_en_map[periph].bit);
}

/**
 * @fn              - GPIOx_BaseAddr_To_Code
 *
 * @brief           - Converts GPIO_RegDef_t pointer into code
 *
 * @param[in]       - GPIOx_RegDef_t* port
 *
 * @return          - uint8_t
 *
 * @Note            - none
 */
static inline uint8_t GPIOx_BaseAddr_To_Code(GPIOx_RegDef_t *port) {
  if (port == GPIOA)
    return 0;
  if (port == GPIOB)
    return 1;
  if (port == GPIOC)
    return 2;
  if (port == GPIOD)
    return 3;
  if (port == GPIOE)
    return 4;
  if (port == GPIOF)
    return 5;
  if (port == GPIOG)
    return 6;
  return 0;
}

/**
 * @fn              - SysTick_Clock_Init
 *
 * @brief           - Initializes SysTick Clock
 *
 * @param[in]       - uint32_t ms (milliseconds)
 *
 * @return          - none
 *
 * @Note            - none
 */
static inline void SysTick_Clock_Init(uint32_t ms) {

  SYSTICK->SYST_CSR = 0; // Turn off the clock

  SYSTICK->SYST_RVR = (CLOCKSPEED / 1000) * ms - 1; // set the reload value

  SYSTICK->SYST_CVR = 0; // clear the current value

  SYSTICK->SYST_CSR |=
      SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSOURCE; // enable the clock
}
static inline void SysTick_Clock_DeInit(void) {

  SYSTICK->SYST_CSR = 0; // Turn off the clock
}

/**
 * @fn              - NVIC_EnableIRQ
 *
 * @brief           - Configures IRQ number,
 *                    enables interrupt
 *
 * @param[in]       - IRQ number
 *
 * @return          - none
 *
 * @Note            - none
 */

static inline void NVIC_EnableIRQ(uint8_t IRQNumber) {

  uint8_t reg_pos = IRQNumber / 32;
  uint8_t bit_pos = IRQNumber % 32;

  // enabling the Interrupt
  NVIC->ISER[reg_pos] = (1 << bit_pos);
}

/**
 * @fn              - NVIC_DisableIRQ
 *
 * @brief           - Configures IRQ number,
 *                    disables interrupt
 *
 * @param[in]       - IRQ number
 *
 * @return          - none
 *
 * @Note            - none
 */
static inline void NVIC_DisableIRQ(uint8_t IRQNumber) {
  uint8_t reg_pos = IRQNumber / 32;
  uint8_t bit_pos = IRQNumber % 32;

  NVIC->ICER[reg_pos] = (1 << bit_pos);
}

/**
 * @fn              - NVIC_SetPriority
 *
 * @brief           - Configures priority of the IRQ

 *
 * @param[in]       - IRQ number
 * @param[in]       - IRQ priority

 *
 * @return          - none
 *
 * @Note            - none
 */
static inline void NVIC_SetPriority(uint8_t IRQNumber, uint8_t priority) {
  uint8_t priority_reg_pos = IRQNumber / 4;
  uint8_t priority_bit_pos = (8 * (IRQNumber % 4) + (8 - NO_PR_BITS_IMPLEMENTED));

  // setting the priority

  NVIC->IPR[priority_reg_pos] &= ~(0xFFU << (8 * (IRQNumber % 4)));
  NVIC->IPR[priority_reg_pos] |= ((uint32_t)priority << priority_bit_pos);
}

/**
 * @fn              - SCB_SetPendSVPriority
 *
 * @brief           - Sets the priority of the PendSV system exception
 *
 * @param[in]       - priority, 0 (highest) to 15 (lowest)
 *
 * @return          - none
 *
 * @Note            - PendSV priority lives in SHPR3 bits 23:16, not in the
 *                    NVIC IPR registers. Keep it lowest.
 */
static inline void SCB_SetPendSVPriority(uint8_t priority) {
  uint32_t byte = ((uint32_t)priority << (8 - NO_PR_BITS_IMPLEMENTED)) & 0xFFU;

  SCB->SHPR3 = (SCB->SHPR3 & ~(0xFFU << 16)) | (byte << 16);
}

/**
 * @fn              - SCB_SetSysTickPriority
 *
 * @brief           - Sets the priority of the SysTick system exception
 *
 * @param[in]       - priority, 0 (highest) to 15 (lowest)
 *
 * @return          - none
 *
 * @Note            - SysTick's priority byte is bits 31:24 of SHPR3.
 */
static inline void SCB_SetSysTickPriority(uint8_t priority) {
  uint32_t byte = ((uint32_t)priority << (8 - NO_PR_BITS_IMPLEMENTED)) & 0xFFU;
  SCB->SHPR3 = (SCB->SHPR3 & ~(0xFFU << 24)) | (byte << 24);
}

/*
 * Generic Macros
 */
#define ENABLE 1U
#define DISABLE 0U
#define SET ENABLE
#define RESET DISABLE

#define GPIO_PIN_SET SET
#define GPIO_PIN_RESET RESET

#define FLAG_SET SET
#define FLAG_RESET RESET

/*
         SPI Specific Register Bitfields macros
*/

#define SPI_CR1_CPHA 0U
#define SPI_CR1_CPOL 1U
#define SPI_CR1_MSTR 2U
#define SPI_CR1_BR 3U
#define SPI_CR1_SPE 6U
#define SPI_CR1_LSBFRST 7U
#define SPI_CR1_SSI 8U
#define SPI_CR1_SSM 9U
#define SPI_CR1_RXONLY 10U
#define SPI_CR1_DFF 11U
#define SPI_CR1_CRCNEXT 12U
#define SPI_CR1_CRCEN 13U
#define SPI_CR1_BIDIOE 14U
#define SPI_CR1_BIDIMOD 15U

#define SPI_CR2_RXDMAEN 0U
#define SPI_CR2_TXDMAEN 1U
#define SPI_CR2_SSOE 2U
#define SPI_CR2_FRF 4U
#define SPI_CR2_ERRIE 5U
#define SPI_CR2_RXNEIE 6U
#define SPI_CR2_TXEIE 7U

#define SPI_SR_RXNE 0U
#define SPI_SR_TXE 1U
#define SPI_SR_CHSIDE 2U
#define SPI_SR_UDR 3U
#define SPI_SR_CRCERR 4U
#define SPI_SR_MODF 5U
#define SPI_SR_OVR 6U
#define SPI_SR_BSY 7U
#define SPI_SR_FRE 8U

#endif /* IRC_STM32F446XX_H_ */
