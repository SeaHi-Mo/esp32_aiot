#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "esp_log.h"
#include "esp_err.h"
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "aiot_dm_api.h"
#include "app_data_md.h"
#include "app_dynreg.h"
static char* TAG = "AA_DATA_MD";

/**
 * @brief app_dm_recv_handler
 *        物模型 事件回调
 * @param dm_handle
 * @param recv
 * @param userdata
 */
static void app_dm_recv_handler(void* dm_handle, const aiot_dm_recv_t* recv, void* userdata)
{
    printf("demo_dm_recv_handler, type = %d\r\n", recv->type);

    switch (recv->type) {

        /* 属性上报, 事件上报, 获取期望属性值或者删除期望属性值的应答 */
        case AIOT_DMRECV_GENERIC_REPLY:
            ESP_LOGI(TAG, "AIOT_DMRECV_GENERIC_REPLY,msg_id = %d, code = %d, data = %.*s, message = %.*s",
            recv->data.generic_reply.msg_id,
            recv->data.generic_reply.code,
            recv->data.generic_reply.data_len,
            recv->data.generic_reply.data,
            recv->data.generic_reply.message_len,
            recv->data.generic_reply.message);
            break;
            /* 云平台属性设置 */
        case AIOT_DMRECV_PROPERTY_SET:
            ESP_LOGI(TAG, "AIOT_DMRECV_PROPERTY_SET,msg_id = %d, code = %d, data = %.*s, message = %.*s",
            recv->data.generic_reply.msg_id,
            recv->data.generic_reply.code,
            recv->data.generic_reply.data_len,
            recv->data.generic_reply.data,
            recv->data.generic_reply.message_len,
            recv->data.generic_reply.message);
            break;

            /* 异步服务调用 */
        case AIOT_DMRECV_ASYNC_SERVICE_INVOKE:


            break;

            /* 同步服务调用 */
        case AIOT_DMRECV_SYNC_SERVICE_INVOKE:


            break;

            /* 下行二进制数据 */
        case AIOT_DMRECV_RAW_DATA:


            break;

            /* 二进制格式的同步服务调用, 比单纯的二进制数据消息多了个rrpc_id */
        case AIOT_DMRECV_RAW_SYNC_SERVICE_INVOKE:


            break;

            /* 上行二进制数据后, 云端的回复报文 */
        case AIOT_DMRECV_RAW_DATA_REPLY:

            break;

        default:
            break;
    }
}
/**
 * @brief app_aiot_data_model_init
 *         初始化物模型
 * @param handle MQTT句柄
 * @return uint32_t
 */
void* app_aiot_data_model_init(void* mqtt_handle)
{

    void* dm_handle = NULL;
    uint8_t post_reply = 1;
    dm_handle = aiot_dm_init();
    if (dm_handle == NULL) {
        ESP_LOGE(TAG, "aiot_dm_init fail");
        return NULL;
    }
    /* 配置MQTT实例句柄 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_MQTT_HANDLE, mqtt_handle);

    /* 配置消息接收处理回调函数 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_RECV_HANDLER, (void*)app_dm_recv_handler);

    /* 配置是云端否需要回复post_reply给设备. 如果为1, 表示需要云端回复, 否则表示不回复 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_POST_REPLY, (void*)&post_reply);

    return dm_handle;
}
/**
 * @brief app_send_property_post
 *      发送温湿度数据到云平台
 * @param dm_handle 物模型句柄 
 * @param temp 温度值
 * @param humi 湿度值
 * @return uint32_t
 */
uint32_t app_send_property_post(void* dm_handle, uint8_t temp, uint8_t humi)
{
    aiot_dm_msg_t app_msg;
    char params[256] = { 0 };
    memset(params, 0, 256);
    memset(&app_msg, 0, sizeof(aiot_dm_msg_t));
    app_msg.type = AIOT_DMMSG_PROPERTY_POST;
    sprintf(params, "{\"Humidity\":%d,\"temperature\":%d}", temp, humi);
    app_msg.data.property_post.params = params;
    return aiot_dm_send(dm_handle, &app_msg);
}
