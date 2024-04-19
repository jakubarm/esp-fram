/**
 * @file fram.h
 *
 * ESP-IDF driver for MB85RC04V I2C FRAM
 *
 * Copyright (c) 2023 Jakub Arm
 * MIT license
 */

#ifndef __FRAM_H__
#define __FRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "driver/i2c.h"

/**
 * @brief FRAM device structure
 *
 * Structure to define the memory device
 */
typedef struct {
    i2c_port_t port;    /**< I2C_NUM_0 or I2C_NUM_1 */
    uint8_t address;    /**< Hardware address of the device */
    uint32_t size;      /**< Size of the memory in bytes */
} fram_t;

/**
 * @brief Initialize the driver
 * @param me Pointer to the device structure
 * @return 'ESP_OK' on success
*/
esp_err_t fram_init(fram_t *me);

/**
 * @brief Reads byte from address
 * @param me Pointer to the device structure
 * @param memAddr Memory address
 * @param d Pointer to a 8 bit to hold value
 * @return 'ESP_OK' on success
*/
esp_err_t fram_read_byte(fram_t *me, uint16_t memAddr, uint8_t *d);

/**
 * @brief Writes byte to address
 * @param me Pointer to the device structure
 * @param memAddr Memory address
 * @param d New value
 * @return 'ESP_OK' on success
*/
esp_err_t fram_write_byte(fram_t *me, uint16_t memAddr, uint8_t d);

/**
 * @brief Reads bytes from address
 * @param me Pointer to the device structure
 * @param memAddr Memory address
 * @param len Data length
 * @param buffer Pointer to byte buffer for reading
 * @return 'ESP_OK' on success
*/
esp_err_t fram_read(fram_t *me, uint16_t memAddr, uint16_t len, uint8_t *buffer);

/**
 * @brief Writes byte to address
 * @param me Pointer to the device structure
 * @param memAddr Memory address
 * @param len Data length
 * @param buffer Pointer to byte buffer for writing
 * @return 'ESP_OK' on success
*/
esp_err_t fram_write(fram_t *me, uint16_t memAddr, uint16_t len, uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif  // __FRAM_H__