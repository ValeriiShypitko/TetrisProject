/*
 * stm32f446xx_spi_driver.c
 *
 *  Created on: Jun 5, 2026
 *      Author: 2k18h
 */

#include "stm32f446xx_spi_driver.h"

/**
 * @fn              - SPI_PeriphEnable
 *
 * @param[in]       - SPI_RegDef_t *pSPIx - pointer to base address of SPI
 * peripheral
 *
 * @param[in]       - clock enum of the SPI peripheral
 *
 * @return          - none
 *
 * @Note            - none
 */

void SPI_PeriphEnable(SPI_RegDef_t *pSPIx) {
  pSPIx->SPI_CR1 |= (1 << SPI_CR1_SPE);
}

/**
 * @fn              - SPI_PeriphDisable
 *
 * @brief           - Disables selected SPI peripheral
 *
 * @param[in]       - SPI_RegDef_t *pSPIx - pointer to base address of SPI
 peripheral

 *
 * @return          - none
 *
 * @Note            - none
 */

void SPI_PeriphDisable(SPI_RegDef_t *pSPIx) {
  pSPIx->SPI_CR1 &= ~(1 << SPI_CR1_SPE);
}
/**
 * @fn              - SPI_Init
 *
 * @brief           - Initializes selected SPI peripheral registers
 *
 * @param[in]       - SPI_Handle_t pointer
 *
 * @return          - none
 *
 * @Note            - none
 */

void SPI_Init(SPI_Handle_t *pSPIHandle) {
  uint32_t temp = 0;

  temp |= pSPIHandle->SPIConfig.SPI_DeviceMode
          << SPI_CR1_MSTR; // Setting device mode master/slave

  if (pSPIHandle->SPIConfig.SPI_BusConfig ==
      SPI_BusConfig_SIMPLEXRX) { // if SPI is SIMPLEX
    temp |= (1 << SPI_CR1_RXONLY);
  } else if (pSPIHandle->SPIConfig.SPI_BusConfig ==
             SPI_BusConfig_HD) { // if SPI is Half-Duplex
    temp |= (1 << SPI_CR1_BIDIMOD);
  }
  // Don't need to configure for full-duplex

  // Configuring clock speed

  temp |= pSPIHandle->SPIConfig.SPI_SCLKSpeed << SPI_CR1_BR;

  // Data format 8bit/16bit

  temp |= pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;

  // Software Slave Management enable or disable
  temp |= pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM;

  // Clock Polarity Setting

  temp |= pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;

  // Clock Phase Setting

  temp |= pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

  pSPIHandle->pSPIx->SPI_CR1 = temp;
}

/**
 * @fn              - SPI_DeInit
 *
 * @brief           - Resets the selected SPI peripheral registers
 *                    to their default reset values
 *
 * @param[in]       - clock enum SPI peripheral
 *
 * @return          - none
 *
 * @Note            - none
 */
void SPI_DeInit(RCC_Periph_e SPIx) {
  *rcc_reset_map[SPIx].reg |= (0x1U << rcc_en_map[SPIx].bit);
  *rcc_reset_map[SPIx].reg &= (0x1U << rcc_en_map[SPIx].bit);
}

/**
 * @fn              - SPI_SendData
 *
 * @brief           - Sends the data
 *
 * @param[in]       - SPI_RegDef_t *pSPIx - pointer to base address of SPI
 * peripheral
 * @param[in]       - uint8_t *pTxBuffer - pointer to buffer
 * @param[in]       - uint32_t len - buffer length in bytes
 *
 * @return          - none
 *
 * @Note            - none
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len) {
  while (len > 0) {
    // 1. Wait until TXBuffer is empty
    while (SPI_GetStatusFlag(pSPIx, SPI_TXE_FLAG) == FLAG_RESET)
      ;
    // 2. Send Data (16 or 8 bits)
    if (pSPIx->SPI_CR1 & (1 << SPI_CR1_DFF)) {
      pSPIx->SPI_DR = *((uint16_t *)pTxBuffer);
      pTxBuffer += 2;
      len = (len >= 2) ? (len - 2) : 0;
    } else {
      pSPIx->SPI_DR = *pTxBuffer;
      pTxBuffer++;
      len--;
    }
    while (SPI_GetStatusFlag(pSPIx, SPI_BSY_FLAG))
      ; /* Wait until SPI is busy in communication or Tx buffer is not empty*/
  }
}

void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len) {
  while (len > 0) {
    // 1. Wait until RXBuffer is empty
    while (SPI_GetStatusFlag(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET)
      ;
    // 2. Receive Data (16 or 8 bits)
    if (pSPIx->SPI_CR1 & (1 << SPI_CR1_DFF)) {
      *((uint16_t *)pRxBuffer) = pSPIx->SPI_DR;
      pRxBuffer += 2;
      len = (len >= 2) ? (len - 2) : 0;

    } else {
      *pRxBuffer = pSPIx->SPI_DR;
      pRxBuffer++;
      len--;
    }
  }
}

void SPI_TransferReceiveData(SPI_RegDef_t *pSPIx, uint8_t *dataOut,
                             uint8_t *dataIn, uint32_t length) {
  for (uint32_t i = 0; i < length; i++) {

    while (SPI_GetStatusFlag(pSPIx, SPI_TXE_FLAG) == FLAG_RESET)
      ; /* Wait until Tx buffer isn't empty*/

    pSPIx->SPI_DR = dataOut[i];
    while (SPI_GetStatusFlag(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET)
      ; /* Wait until Rx buffer is empty*/

    dataIn[i] = pSPIx->SPI_DR;

    while (SPI_GetStatusFlag(pSPIx, SPI_BSY_FLAG))
      ; /* Wait until SPI is busy in communication or Tx buffer is not empty*/
  }
}

/**
 * @brief   Transfers one byte over SPI while simultaneously receiving a byte.
 * @details Full-duplex operation: while @p data is shifted out on MOSI, a byte
 *          is shifted in from MISO and returned. Thin wrapper around
 *          SPI_TransmitReceive() for convenient use with the MFRC522.
 * @param   SPI register address.
 * @param   data  The byte sent to the slave (MFRC522).
 * @return  The byte received from the MFRC522 during the same transfer.
 */

uint8_t SPI_Transfer(SPI_RegDef_t *pSPIx, uint8_t data) {

  uint8_t rxData;
  SPI_TransferReceiveData(pSPIx, &data, &rxData, 1);
  return rxData;
}

uint8_t SPI_GetStatusFlag(SPI_RegDef_t *pSPIx, uint32_t flagName) {
  if (pSPIx->SPI_SR & flagName) {
    return FLAG_SET;
  }
  return FLAG_RESET;
}

void SPI2_GPIOs_Init(void) {
  GPIO_Handle_t gpioSPI2pins;
  memset(&gpioSPI2pins, 0, sizeof(GPIO_Handle_t));
  gpioSPI2pins.GPIOx = GPIOB;
  gpioSPI2pins.PinConfig.GPIOx_PinMode = GPIO_MODE_OUT;
  gpioSPI2pins.PinConfig.GPIOx_PinAltFuncMode = 5;
  gpioSPI2pins.PinConfig.GPIOx_PinOutputType = GPIO_OT_PP;
  gpioSPI2pins.PinConfig.GPIOx_PinPuPdControl = GPIO_NOPUD;
  gpioSPI2pins.PinConfig.GPIOx_PinSpeed = GPIO_OS_HS;

  gpioSPI2pins.PinConfig.GPIOx_PinNumber = GPIO_PIN_12; // NSS config
  RCC_Enable_Clock(RCC_GPIOB);
  GPIO_Init(&gpioSPI2pins);
  gpioSPI2pins.PinConfig.GPIOx_PinMode = GPIO_MODE_ALTFN;
  gpioSPI2pins.PinConfig.GPIOx_PinNumber = GPIO_PIN_13; // SCK config
  GPIO_Init(&gpioSPI2pins);

  gpioSPI2pins.PinConfig.GPIOx_PinNumber = GPIO_PIN_14; // MISO config
  GPIO_Init(&gpioSPI2pins);

  gpioSPI2pins.PinConfig.GPIOx_PinNumber = GPIO_PIN_15; // MOSI config
  GPIO_Init(&gpioSPI2pins);
}

void SPI_Pull_CS_LOW(GPIOx_RegDef_t *GPIO_Port, uint8_t GPIO_Pin) {
  GPIO_WriteToOutputPin(GPIO_Port, GPIO_Pin, RESET); // CS LOW
}

void SPI_Pull_CS_HIGH(GPIOx_RegDef_t *GPIO_Port, uint8_t GPIO_Pin) {
  GPIO_WriteToOutputPin(GPIO_Port, GPIO_Pin, SET); // CS HIGH
}

void SPI2_Init(void) {
  SPI2_GPIOs_Init();
  SPI_Handle_t SPI2Handle;
  memset(&SPI2Handle, 0, sizeof(SPI_Handle_t));
  SPI2Handle.pSPIx = SPI2;
  SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DeviceMode_Master;
  SPI2Handle.SPIConfig.SPI_SCLKSpeed = SPI_SCLKSpeed_DIV2;
  SPI2Handle.SPIConfig.SPI_DFF = SPI_DFF_8Bits;
  SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_EN;
  SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_0;
  SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LEAD;
  SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BusConfig_FD;
  RCC_Enable_Clock(RCC_SPI2);
  SPI_Init(&SPI2Handle);
}

void SPI2_SSI_Config(SPI_RegDef_t *pSPIx) {
  pSPIx->SPI_CR1 |= 1 << SPI_CR1_SSI;
}
