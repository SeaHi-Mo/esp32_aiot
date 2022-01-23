#ifndef APP_DYNREGMQ_H
#define APP_DYNREGMQ_H

//如无需使用动态注册，就把这个宏屏蔽掉
#define APP_DYNREG_ENABLE
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_dynregmq_api.h"
#ifdef APP_DYNREG_ENABLE

typedef struct {
    char conn_clientid[128];
    char conn_username[128];
    char conn_password[64];
    char device_secret[64];
}cloud_device_wl_t;

extern char* product_key;
extern char device_name[13];
extern char* product_secret;
extern char* url;
extern uint16_t    port;
extern cloud_device_wl_t cloud_device_wl;
int dynregmq_start(cloud_device_wl_t* cloud_dvc, aiot_sysdep_network_cred_t cred);
#endif

#endif