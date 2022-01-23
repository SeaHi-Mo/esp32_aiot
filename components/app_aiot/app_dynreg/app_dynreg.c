#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "app_dynreg.h"
static const char* TAG = "dynregmq device";

#ifdef APP_DYNREG_ENABLE

//请改成自己产品的三元组

char* product_key = "a1y36ql6F5x";
char device_name[13] = { '\0' };
char* product_secret = "aNnFWOPOrNEGG59Z";
char* url = "iot-as-mqtt.cn-shanghai.aliyuncs.com"; /* 阿里云平台上海站点的域名后缀 */
uint16_t    port = 443;      /* 无论设备是否使用TLS连接阿里云平台, 目的端口都是443 */
cloud_device_wl_t cloud_device_wl;
uint8_t skip_pre_regist = 1;        /* TODO: 如果要免预注册, 需要将该值设置为1;如果需要在控制台预先注册设备, 置为0 */
/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 数据处理回调, 当SDK从网络上收到dynregmq消息时被调用 */
void demo_dynregmq_recv_handler(void* handle, const aiot_dynregmq_recv_t* packet, void* userdata)
{
    switch (packet->type) {
        /* TODO: 回调中需要将packet指向的空间内容复制保存好, 因为回调返回后, 这些空间就会被SDK释放 */
        case AIOT_DYNREGMQRECV_DEVICEINFO_WL: {
            if (strlen(packet->data.deviceinfo_wl.device_secret) >= sizeof(cloud_device_wl.device_secret)) {
                break;
            }

            /* 白名单模式, 用户务必要对device_secret进行持久化保存 */
            memset(&cloud_device_wl, 0, sizeof(cloud_device_wl_t));
            memcpy(cloud_device_wl.device_secret, packet->data.deviceinfo_wl.device_secret,
                   strlen(packet->data.deviceinfo_wl.device_secret));
        }
                                            break;
                                            /* TODO: 回调中需要将packet指向的空间内容复制保存好, 因为回调返回后, 这些空间就会被SDK释放 */
        case AIOT_DYNREGMQRECV_DEVICEINFO_NWL: {
            if (strlen(packet->data.deviceinfo_nwl.clientid) >= sizeof(cloud_device_wl.conn_clientid) ||
                strlen(packet->data.deviceinfo_nwl.username) >= sizeof(cloud_device_wl.conn_username) ||
                strlen(packet->data.deviceinfo_nwl.password) >= sizeof(cloud_device_wl.conn_password)) {
                break;
            }

            /* 免白名单模式, 用户务必要对MQTT的建连信息clientid, username和password进行持久化保存 */
            memset(&cloud_device_wl, 0, sizeof(cloud_device_wl_t));
            memcpy(cloud_device_wl.conn_clientid, packet->data.deviceinfo_nwl.clientid, strlen(packet->data.deviceinfo_nwl.clientid));
            memcpy(cloud_device_wl.conn_username, packet->data.deviceinfo_nwl.username, strlen(packet->data.deviceinfo_nwl.username));
            memcpy(cloud_device_wl.conn_password, packet->data.deviceinfo_nwl.password, strlen(packet->data.deviceinfo_nwl.password));

        }
                                             break;
        default: {

        }
               break;
    }
}
/**
 * @brief 执行动态注册
 *
 * @param cloud_dvc
 */
int dynregmq_start(cloud_device_wl_t* cloud_dvc, aiot_sysdep_network_cred_t cred)
{
    int32_t     res = STATE_SUCCESS;
    void* dynregmq_handle = NULL;
    /* 创建1个dynregmq客户端实例并内部初始化默认参数 */

    char host[128] = { 0 };

    dynregmq_handle = aiot_dynregmq_init();
    if (dynregmq_handle == NULL) {
        printf("aiot_dynregmq_init failed\n");

        return -1;
    }
    //  sprintf(host, "%s.%s", product_key, url);
      /* 配置连接的服务器地址 */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_HOST, (void*)url);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_HOST failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);
        return -1;
    }

    /* 配置连接的服务器端口 */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_PORT, (void*)&port);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_PORT failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 配置设备productKey */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_PRODUCT_KEY, (void*)product_key);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_PRODUCT_KEY failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 配置设备productSecret */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_PRODUCT_SECRET, (void*)product_secret);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_PRODUCT_SECRET failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 配置设备deviceName */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_DEVICE_NAME, (void*)device_name);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_DEVICE_NAME failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_NETWORK_CRED, (void*)&cred);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_NETWORK_CRED failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 配置DYNREGMQ默认消息接收回调函数 */
    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_RECV_HANDLER, (void*)demo_dynregmq_recv_handler);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_RECV_HANDLER failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    res = aiot_dynregmq_setopt(dynregmq_handle, AIOT_DYNREGMQOPT_NO_WHITELIST, (void*)&skip_pre_regist);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_setopt AIOT_DYNREGMQOPT_NO_WHITELIST failed, res: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 发送动态注册请求 */
    res = aiot_dynregmq_send_request(dynregmq_handle);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_send_request failed: -0x%04X\n\r\n", -res);
        printf("please check variables like mqtt_host, produt_key, device_name, product_secret in demo\r\n");
        aiot_dynregmq_deinit(&dynregmq_handle);

        return -1;
    }

    /* 接收动态注册请求 */
    res = aiot_dynregmq_recv(dynregmq_handle);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_recv failed: -0x%04X\n", -res);
        aiot_dynregmq_deinit(&dynregmq_handle);
        return -1;
    }
    if (skip_pre_regist == 0) {
        printf("device secret: %s\n", cloud_device_wl.device_secret);
    }
    else {
        printf("clientid: %s\n", cloud_device_wl.conn_clientid);
        printf("username: %s\n", cloud_device_wl.conn_username);
        printf("password: %s\n", cloud_device_wl.conn_password);


    }
    /* 销毁动态注册会话实例 */
    res = aiot_dynregmq_deinit(&dynregmq_handle);
    if (res < STATE_SUCCESS) {
        printf("aiot_dynregmq_deinit failed: -0x%04X\n", -res);

        return -1;
    }
    return 0;
}

#endif