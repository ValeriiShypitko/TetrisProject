/*
 * stm32f446xx_spi_driver.h
 *
 *  Created on: Jun 5, 2026
 *      Author: 2k18h
 */

#ifndef IRC_STM32F446XX_SPI_DRIVER_H_
#define IRC_STM32F446XX_SPI_DRIVER_H_

#include "stm32f446xx.h"

#define SPI_RXNE_FLAG (1U << SPI_SR_RXNE)
#define SPI_TXE_FLAG (1U << SPI_SR_TXE)
#define SPI_CHSIDE_FLAG (1U << SPI_SR_CHSIDE)
#define SPI_UDR_FLAG (1U << SPI_SR_UDR)
#define SPI_CRCERR_FLAG (1U << SPI_SR_CRCERR)
#define SPI_MODF_FLAG (1U << SPI_SR_MODF)
#define SPI_OVR_FLAG (1U << SPI_SR_OVR)
#define SPI_BSY_FLAG (1U << SPI_SR_BSY)
#define SPI_FRE_FLAG (1U << SPI_SR_FRE)

typedef struct {
  uint8_t SPI_DeviceMode; // @SPI_DeviceMode
  uint8_t SPI_BusConfig;  // @SPI_BusConfig
  uint8_t SPI_SCLKSpeed;  // @SPI_SCLKSpeed
  uint8_t SPI_DFF;        // @SPI_DFF
  uint8_t SPI_CPOL;       // @SPI_CPOL
  uint8_t SPI_CPHA;       // @SPI_CPHA
  uint8_t SPI_SSM;        // @SPI_SSM
} SPI_Config_t;

typedef struct {
  SPI_RegDef_t *pSPIx;
  SPI_Config_t SPIConfig;
} SPI_Handle_t;

/*=======================
        SPI Macros
 ========================*/

// @SPI_DeviceMode
#define SPI_DeviceMode_Slave 0U
#define SPI_DeviceMode_Master 1U

// @SPI_BusConfig
#define SPI_BusConfig_FD 0U
#define SPI_BusConfig_HD 1U
#define SPI_BusConfig_SIMPLEXRX 2U

// @SPI_SCLKSpeed
#define SPI_SCLKSpeed_DIV2 0U
#define SPI_SCLKSpeed_DIV4 1U
#define SPI_SCLKSpeed_DIV8 2U
#define SPI_SCLKSpeed_DIV16 3U
#define SPI_SCLKSpeed_DIV32 4U
#define SPI_SCLKSpeed_DIV64 5U
#define SPI_SCLKSpeed_DIV128 6U
#define SPI_SCLKSpeed_DIV256 7U

// @SPI_DFF
#define SPI_DFF_8Bits 0U
#define SPI_DFF_16Bits 1U

// @SPI_CPOL
#define SPI_CPOL_0 0U
#define SPI_CPOL_1 1U

// @SPI_CPHA
#define SPI_CPHA_LEAD 0U
#define SPI_CPHA_TRAIL 1U

// @SPI_SSM
#define SPI_SSM_DIS 0U
#define SPI_SSM_EN 1U

/*=======================
        API prototypes
 ========================*/

/*============================================
         SPIx enable and disable
 =============================================*/

void SPI_PeriphEnable(SPI_RegDef_t *pSPIx);
void SPI_PeriphDisable(SPI_RegDef_t *pSPIx);
/*=======================
        SPIx Init-DeInit
 ========================*/

void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(RCC_Periph_e SPIx);

/*==========================
        SPIx Send-Receive data
 ===========================*/

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len);

uint8_t SPI_Transfer(SPI_RegDef_t *pSPIx, uint8_t data);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len);
void SPI_TransferReceiveData(SPI_RegDef_t *pSPIx, uint8_t *dataOut,
                             uint8_t *dataIn, uint32_t length);

/*============================================
         SPIx ISR handling
 =============================================*/

void SPI_IRQHandling(uint8_t SPINumber);

/*============================================
         SPIx get Flag
 =============================================*/

uint8_t SPI_GetStatusFlag(SPI_RegDef_t *pSPIx, uint32_t flagName);
// void SPI_TransferData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint8_t
// *pRxBuffer, uint32_t len);
void SPI_TransferData(SPI_RegDef_t *pSPIx, uint8_t data);

void SPI2_GPIOs_Init(void);
void SPI2_Init(void);
void SPI2_SSI_Config(SPI_RegDef_t *pSPIx);
void SPI_Pull_CS_LOW(GPIOx_RegDef_t *GPIO_Port, uint8_t GPIO_Pin);
void SPI_Pull_CS_HIGH(GPIOx_RegDef_t *GPIO_Port, uint8_t GPIO_Pin);

#endif /* IRC_STM32F446XX_SPI_DRIVER_H_ */
