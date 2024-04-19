/*
 * fram.c
 * 2023 Jakub Arm
*/
#include "fram.h"

#include <esp_err.h>
#include <esp_log.h>

#include "driver/i2c.h"


// registers


#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

#define ACK_CHECK_EN 0x1       /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0      /*!< I2C master will not check ack from slave */
#define ACK_VAL    0x0         /*!< I2C ack value */
#define NACK_VAL   0x1         /*!< I2C nack value */

static const char *TAG = "FRAM";

// Functions


esp_err_t fram_init(fram_t *me) {
    ESP_LOGI(TAG, "Init FRAM port %d address %d", me->port, me->address);
    return ESP_OK;
}

esp_err_t fram_read_byte(fram_t *me, uint16_t memAddr, uint8_t *d) {
    CHECK_ARG(me);

    if( memAddr >= me->size ) {
        ESP_LOGE(TAG, "Error reading address %d (size)", memAddr);
        return ESP_FAIL;
    }

    uint8_t page = ((memAddr >> 8) & 0xFF);

    ESP_LOGI(TAG, "Read byte port %d address %d memAddr %d", me->port, me->address, memAddr);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (me->address << 1) | (page << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (memAddr & 0xFF), ACK_CHECK_EN);
    i2c_master_read_byte(cmd, d, NACK_VAL);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(me->port, cmd, pdMS_TO_TICKS(CONFIG_FRAM_COMMAND_TIMEOUT));
    i2c_cmd_link_delete(cmd);
    if( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Error reading address %d (error %d)", memAddr, ret);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t fram_write_byte(fram_t *me, uint16_t memAddr, uint8_t d) {
    CHECK_ARG(me);

    if( memAddr >= me->size ) {
        ESP_LOGE(TAG, "Error writing address %d (size)", memAddr);
        return ESP_FAIL;
    }

    uint8_t page = ((memAddr >> 8) & 0xFF);

    ESP_LOGI(TAG, "Write byte port %d address %d memAddr %d value %d", me->port, me->address, memAddr, d);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (me->address << 1) | (page << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (memAddr & 0xFF), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, d, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(me->port, cmd, pdMS_TO_TICKS(CONFIG_FRAM_COMMAND_TIMEOUT));
    i2c_cmd_link_delete(cmd);
    if( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Error writing address %d (error %d)", memAddr, ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t fram_read(fram_t *me, uint16_t memAddr, uint16_t len, uint8_t *buffer) {
    CHECK_ARG(me);

    if( memAddr >= me->size ) {
        ESP_LOGE(TAG, "Error reading address %d (size)", memAddr);
        return ESP_FAIL;
    }

    uint8_t page = ((memAddr >> 8) & 0xFF);

    ESP_LOGI(TAG, "Read port %d address %d memAddr %d len %d", me->port, me->address, memAddr, len);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (me->address << 1) | (page << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (memAddr & 0xFF), ACK_CHECK_EN);
    for (uint16_t i = 0; i < (len - 1); i++) {
        i2c_master_read_byte(cmd, &(buffer[i]), ACK_VAL);
    }
    i2c_master_read_byte(cmd, &(buffer[len - 1]), NACK_VAL);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(me->port, cmd, pdMS_TO_TICKS(CONFIG_FRAM_COMMAND_TIMEOUT));
    i2c_cmd_link_delete(cmd);
    if( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Error reading address %d (error %d)", memAddr, ret);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t fram_write(fram_t *me, uint16_t memAddr, uint16_t len, uint8_t *buffer) {
    CHECK_ARG(me);

    if( memAddr >= me->size ) {
        ESP_LOGE(TAG, "Error writing address %d (size)", memAddr);
        return ESP_FAIL;
    }

    uint8_t page = ((memAddr >> 8) & 0xFF);

    ESP_LOGI(TAG, "Write port %d address %d memAddr %d len %d", me->port, me->address, memAddr, len);
    uint16_t current = memAddr;
    while(current < (memAddr + len))
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (me->address << 1) | (page << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, (uint8_t)(current & 0xFF), ACK_CHECK_EN);
        for (uint16_t i = 0; i < CONFIG_FRAM_PAGE_SIZE; i++) {
            if ((current + i) < (memAddr + len))
                i2c_master_write_byte(cmd, buffer[current + i - memAddr], ACK_CHECK_EN);
        }
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(me->port, cmd, pdMS_TO_TICKS(CONFIG_FRAM_COMMAND_TIMEOUT));
        i2c_cmd_link_delete(cmd);
        if( ret != ESP_OK ) {
            ESP_LOGE(TAG, "Error writing address %d (error %d)", current, ret);
            return ESP_FAIL;
        }
        current += CONFIG_FRAM_PAGE_SIZE;
        vTaskDelay(pdMS_TO_TICKS(CONFIG_FRAM_WAIT_TIME));
    }

    return ESP_OK;
}