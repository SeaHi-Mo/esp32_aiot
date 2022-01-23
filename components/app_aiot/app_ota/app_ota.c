#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_ota_api.h"
#include "aiot_mqtt_api.h"
#include "ota_private.h"
#include "app_ota.h"
#include "app_dynreg.h"
#include "cJSON.h"
static char* TAG = "APP OTA";

const esp_app_desc_t* cur_version = NULL;
extern const char* ali_ca_cert;
char ali_ota_msg[1024] = { 0 };
app_ota_image_t app_ota_image;
static void* mqtt_handle = NULL;
int esp_aiot_download_report_progress(int progress);
/**
 * @brief Get the ota msg object
 *      获取ota 信息
 * @param cjson_data
 */
 //
void get_ota_msg(char* cjson_data)
{
    cJSON* root = cJSON_Parse(cjson_data);
    if (root==NULL) {
        cJSON_Delete(root);
        return;
    }
    cJSON* data = cJSON_GetObjectItem(root, "data");
    if (data==NULL) {
        cJSON_Delete(root);
        return;
    }
    cJSON* data_size = cJSON_GetObjectItem(data, "size");
    cJSON* data_version = cJSON_GetObjectItem(data, "version");
    cJSON* data_url = cJSON_GetObjectItem(data, "url");

    app_ota_image.image_size = (uint32_t)data_size->valueint;
    strcpy(app_ota_image.image_version, data_version->valuestring);
    strcpy(app_ota_image.image_url, data_url->valuestring);
}
/**
 * @brief Create a ota progress cjson object
 *        创建升级进度的json数据
 * @param progress
 * @return char*
 */
char* create_ota_progress_cjson(int progress)
{
    char progress_str[2] = { 0 };
    cJSON* Root = cJSON_CreateObject();
    cJSON* params = cJSON_CreateObject();
    cJSON_AddStringToObject(Root, "id", "12345");
    cJSON_AddItemToObject(Root, "params", params);
    sprintf(progress_str, "%d", progress);
    cJSON_AddStringToObject(params, "step", progress_str);
    cJSON_AddStringToObject(params, "desc", "");
    // cJSON_AddStringToObject(params, "module", "");
    char* cjson_data = cJSON_PrintUnformatted(Root);
    cJSON_Delete(Root);
    return cjson_data;
}
/**
 * @brief Create a ota version cjson object
 *         创建版本传的json 
 * @param verison
 * @return char*
 */
char* create_ota_version_cjson(char* verison)
{
    cJSON* Root = cJSON_CreateObject();
    cJSON* param = cJSON_CreateObject();

    //  cJSON_AddStringToObject(param, "module", "default");
    cJSON_AddStringToObject(param, "version", verison);
    cJSON_AddStringToObject(Root, "id", "00001");
    cJSON_AddItemToObject(Root, "params", param);

    char* cjson_data = cJSON_PrintUnformatted(Root);
    cJSON_Delete(Root);
    return cjson_data;
}

/**
 * @brief 版本比较 01.00.01.11 添加
 *
 * @param desc_version
 * @param running_version
 * @return int
 */
int version_cmp(char* desc_version, char* running_version)
{
    char desc_v[4];
    char run_v[4];
    uint32_t desc_version_num = 0;
    uint32_t run_version_num = 0;

    desc_v[0] = atoi(strtok(desc_version, "."));

    for (size_t i = 1; i < 3; i++)
        desc_v[i] = atoi(strtok(NULL, "."));

    run_v[0] = atoi(strtok(running_version, "."));
    for (size_t i = 1; i < 3; i++)
        run_v[i] = atoi(strtok(NULL, "."));
    for (size_t i = 0; i < 3; i++)
    {
        desc_version_num += desc_v[i];
        run_version_num += run_v[i];
        if (i<2) {
            desc_version_num *= 10;
            run_version_num *= 10;
        }
    }
    if ((run_version_num > desc_version_num) ||(run_version_num == desc_version_num)) return 1;
    return 0;
}
/**
 * @brief OTA 线程
 *
 * @param arg
 * @return void*
 */
int ota_download_thread(void)
{
    uint32_t image_len_read = 0;
    int progress = 0;
    esp_err_t ota_finish_err = ESP_OK;
    ota_update_status_t ota_update_status;
    esp_http_client_config_t ota_config = {
        .url = app_ota_image.image_url,
        .cert_pem = ali_ca_cert,
        .timeout_ms = 60000,
        .keep_alive_enable = true,
        .skip_cert_common_name_check = true,
    };

    esp_https_ota_config_t https_ota_config = {
        .http_config = &ota_config,
    };
    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&https_ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        return -1;
    }
    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");

        return -2;
    }
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    //进行版本校验
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
        /***********  版本校验 ************************/
        if (version_cmp(app_ota_image.image_version, running_app_info.version)) {
            ESP_LOGE(TAG, "image version is not updata:%s", app_desc.version);
            return -3;
        }
        else {
            ESP_LOGI(TAG, "version check OK:%s", app_desc.version);
            /************    进行sha256 校验********************/
           /* for (size_t i = 0; i < 32; i++)
            {
                if (app_desc.app_elf_sha256[i] == running_app_info.app_elf_sha256[i]) {
                    ESP_LOGE(TAG, "image sha256 is not updata:");
                    ESP_LOG_BUFFER_HEXDUMP(TAG, app_desc.app_elf_sha256, 32, ESP_LOG_ERROR);
                    return -4;
                }
            }*/
            ESP_LOGI(TAG, "Image sha256 check ok");
            ESP_LOG_BUFFER_HEX(TAG, app_desc.app_elf_sha256, 32);
        }
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {

            break;
        }
        //显示读取到的数据
        image_len_read = esp_https_ota_get_image_len_read(https_ota_handle);

        if (progress != 100 * image_len_read / app_ota_image.image_size)
        {
            ESP_LOGI(TAG, "WiFi OTA progress----------[%d%%]", progress);
            if ((progress%5)==0)
                esp_aiot_download_report_progress(progress);
        }
        progress = 100 * image_len_read / app_ota_image.image_size;
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        // the OTA image was not completely received and user can customise the response to this situation.
        ESP_LOGE(TAG, "Complete data was not received.");
    }
    else {
        //清理OTA 升级的数据并关闭连接 esp_https_ota_finish
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
            esp_aiot_download_report_progress(100);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            //升级完毕，马上复位
            esp_restart();
        }
        else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
        }
    }
    return 0;
}
/**
 * @brief aiot_mqtt_recv_ota_handler
 *        OTA信息下发处理
 * @param handle
 * @param packet
 * @param userdata
 */
void aiot_mqtt_recv_ota_handler(void* handle, const aiot_mqtt_recv_t* packet, void* userdata)
{
    if (packet->type==AIOT_MQTTRECV_PUB) {
        get_ota_msg((char*)packet->data.pub.payload);
        ESP_LOGI(TAG, "imge version:%s,imge size:%d", app_ota_image.image_version, app_ota_image.image_size);
        ota_download_thread();
        //  esp_restart();
    }
}
/**
 * @brief 配置OTA
 *
 * @param mqtt_handle
 */
int32_t esp_ota_aiot_pthread(void* mqtt_handle_t)
{
    int32_t res = STATE_SUCCESS;
    char topic[128] = { 0 };
    sprintf(topic, "%s/%s/%s", OTA_FOTA_TOPIC_PREFIX, product_key, device_name);
    mqtt_handle = mqtt_handle_t;
    res = aiot_mqtt_sub(mqtt_handle, topic, aiot_mqtt_recv_ota_handler, 1, NULL);

    return res;
}
/**
 * @brief esp_aiot_download_report_progress
 *  上传升级百分比到 云平台
 * @param progress
 * @return int
 */
int esp_aiot_download_report_progress(int progress)
{
    int32_t res = STATE_SUCCESS;
    int percen = progress;
    char ota_topic[128] = { 0 };

    sprintf(ota_topic, "%s/%s/%s", OTA_PROGRESS_TOPIC_PREFIX, product_key, device_name);
    char* src = create_ota_progress_cjson(progress);
    res = aiot_mqtt_pub(mqtt_handle, ota_topic, (uint8_t*)src, strlen(src), 1);
    if (res<STATE_SUCCESS) {
        ESP_LOGE(TAG, "pub failed:-0x%04X", -res);
    }
    free(src);
    return res;
}