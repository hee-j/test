#ifndef __K_CONFIG_
#define __K_CONFIG_

#include "mbed.h"

// SD CARD
#define SD_CARD_SPI_MOSI    PC_12
#define SD_CARD_SPI_MISO    PC_11
#define SD_CARD_SPI_SCK     PC_10
#define SD_CARD_SPI_CS      PC_4
#define SD_CARD_DET         PB_7

// UART
#define PC_TX               PA_2
#define PC_RX               PA_3

// RADIO
#define RADIO_RST           PB_1
#define RADIO_MOSI          PB_15
#define RADIO_MISO          PB_14
#define RADIO_SCLK          PB_13
#define RADIO_CS            PC_2
#define RADIO_DIO0          PB_2

// 9DOF IMU
#define IMU_I2C_SDA         D14
#define IMU_I2C_SCL         D15

// Arducam with OV2640
// OV2640
#define CAM_I2C_SDA         PB_4
#define CAM_I2C_SCL         PA_8

// CAM Shield
#define CAM_SPI_MOSI        PA_7
#define CAM_SPI_MISO        PA_6
#define CAM_SPI_SCK         PA_5
#define CAM_SPI_CS          PA_15
#define CAM_EN              PC_0

// BMP280
#define BMP_SDA             PB_3
#define BMP_SCL             PB_10
#define BMP_addr            0x77

// BUZZER
#define BUZZER_PIN          PA_13

// GPS
#define GPS_TX              PC_6
#define GPS_RX              PC_7

// LED DRIVER
#define LED_DRIVER_CS       PA_11
#define LED_DRIVER_SPI_SCLK PA_5 // Be careful with this!
#define LED_DRIVER_MOSI     PA_7

// BT
#define BT_CS               PB_12
#define BT_INT              PA_14
#define BT_RST              PC_14

// RBF
#define RBF                 PC_8

// External payload interface
#define EXT_TX              PB_6
#define EXT_RX              PB_7

// MCP
#define MCP_SPI_MOSI        PB_15
#define MCP_SPI_MISO        PB_14
#define MCP_SPI_SCK         PB_13
#define MCP_CS              PC_9

#define CHARGING_STATUS     PC_5

#endif