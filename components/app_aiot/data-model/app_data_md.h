#ifndef APP_DATA_MD_H
#define APP_DATA_MD_H



void* app_aiot_data_model_init(void* mqtt_handle);
uint32_t app_send_property_post(void* dm_handle, uint8_t temp, uint8_t humi);
#endif
