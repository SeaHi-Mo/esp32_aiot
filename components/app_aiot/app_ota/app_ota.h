#ifndef APP_OTA_H
#define APP_OTA_H
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
typedef struct {
    uint32_t image_size;
    char image_url[1024];
    char image_version[32];
}app_ota_image_t;

typedef enum {
    OTA_UPDATE_DEF = 0,//默认状态
    OTA_UPDATE_SUCCEEDED,//升级成功
    OTA_UPDATE_FAILED,//升级失败
    OTA_UPDATE_CANCELED,//写入失败
}ota_update_status_t;

extern char ali_ota_msg[1024];
extern const esp_app_desc_t* cur_version;

int32_t esp_ota_aiot_pthread(void* mqtt_handle);

#endif
