/**
 * @file app_flash.c
 * @author your name (you@domain.com)
 * @brief   flash 操作复位不还原
 * @version 0.1
 * @date 2022-01-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "app_dynreg.h"
static char* TAG = "APP FLASH";

/**
 * @brief 保存动态注册的信息
 *
 */
esp_err_t app_flash_svae_dynreg_msg(void)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t dynreg_handle;
    err = nvs_open("nvs", NVS_READWRITE, &dynreg_handle);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs open failse");
        return ESP_FAIL;
    }
    err = nvs_set_str(dynreg_handle, "clientID", cloud_device_wl.conn_clientid);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs svae clientID fail");
        nvs_close(dynreg_handle);
        return ESP_FAIL;
    }
    err = nvs_set_str(dynreg_handle, "userName", cloud_device_wl.conn_username);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs svae userName fail");
        nvs_close(dynreg_handle);
        return ESP_FAIL;
    }
    err = nvs_set_str(dynreg_handle, "password", cloud_device_wl.conn_password);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs svae password fail");
        nvs_close(dynreg_handle);
        return ESP_FAIL;
    }
    nvs_close(dynreg_handle);
    ESP_LOGI(TAG, "svae dynreg OK");
    return err;
}
/**
 * @brief app_flash_read_dynreg
 *         读取连接信息
 * @param cloud_device
 * @return esp_err_t
 */
esp_err_t app_flash_read_dynreg(cloud_device_wl_t* cloud_device)
{
    esp_err_t err = ESP_OK;
    nvs_handle_t dynreg_handle;
    char data[128] = { 0 };
    err = nvs_open("nvs", NVS_READONLY, &dynreg_handle);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs open failse");
        return ESP_FAIL;
    }
    size_t size_len = 0;
    memset(data, 0, 128);
    err = nvs_get_str(dynreg_handle, "clientID", data, &size_len);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs get clientID fail");
        nvs_close(dynreg_handle);
        return ESP_ERR_NO_MEM;
    }
    memset(cloud_device->conn_clientid, 0, 128);
    strncpy((char*)cloud_device->conn_clientid, data, size_len);

    memset(data, 0, 128);
    err = nvs_get_str(dynreg_handle, "userName", data, &size_len);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs get userName fail");
        nvs_close(dynreg_handle);
        return ESP_ERR_NO_MEM;
    }
    memset(cloud_device->conn_username, 0, 128);
    strncpy((char*)cloud_device->conn_username, data, size_len);

    memset(data, 0, 128);
    err = nvs_get_str(dynreg_handle, "password", data, &size_len);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG, "nvs get password fail");
        nvs_close(dynreg_handle);
        return ESP_ERR_NO_MEM;
    }
    memset(cloud_device->conn_password, 0, 64);
    strncpy((char*)cloud_device->conn_password, data, size_len);
    nvs_close(dynreg_handle);
    return err;
}
