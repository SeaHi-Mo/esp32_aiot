#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "esp_log.h"
#include "esp_err.h"
#include "ota_private.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "app_version.h"
#include "cJSON.h"
#include "app_dynreg.h"

static char* TAG = "APP_SERSION";

#ifndef APP_DYNREG_ENABLE
extern char* product_key;
extern char* device_name;
extern char* device_secret;
#endif

/**
 * @brief Create a ota version cjson object
 *         创建版本传的json 
 * @param verison
 * @return char*
 */
static char* create_ota_version_cjson(char* verison)
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
 * @brief app_send_new_version
 *        上报version 到云平台
 * @param mqtt_handle
 * @return int32_t
 */
int32_t app_send_new_version(void* mqtt_handle)
{
    int32_t res = STATE_SUCCESS;
    char ota_topic[128] = { 0 };
    esp_app_desc_t* cur_version = NULL;
    sprintf(ota_topic, "%s/%s/%s", OTA_VERSION_TOPIC_PREFIX, product_key, device_name);
    cur_version = esp_ota_get_app_description();
    char* src = create_ota_version_cjson((char*)cur_version->version);
    res = aiot_mqtt_pub(mqtt_handle, ota_topic, (uint8_t*)src, strlen(src), 1);
    if (res<STATE_SUCCESS) {
        ESP_LOGE(TAG, "pub failed:-0x%04X", -res);
    }
    free(src);
    return res;
}