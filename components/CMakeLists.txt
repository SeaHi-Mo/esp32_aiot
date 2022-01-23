set(include_dirs 
    LinkSDK/core 
    LinkSDK/core/sysdep 
    LinkSDK/core/utils 
    LinkSDK/components/data-model
    LinkSDK/components/dynreg
    LinkSDK/components/dynreg-mqtt
    LinkSDK/components/ntp
    LinkSDK/components/ota
    app_aiot/data-model
    app_aiot/app_version
    app_aiot/app_ntp
    app_aiot/app_ota
    app_aiot/app_dynreg
    app_aiot/app_flash
)

file(GLOB c_sdk_srcs
    "LinkSDK/core/*.c"
    "LinkSDK/core/utils/*.c"
    "LinkSDK/core/sysdep/*.c"
    "LinkSDK/portfiles/aiot_port/*.c"
    "LinkSDK/external/*.c"
    "LinkSDK/components/data-model/*.c"
    "LinkSDK/components/dynreg/*.c"
    "LinkSDK/components/dynreg-mqtt/*.c"
    "LinkSDK/components/ntp/*.c"
    "LinkSDK/components/ota/*.c"
    "app_aiot/data-model/*.c"
    "app_aiot/app_version/*.c"
    "app_aiot/app_ntp/*.c"
    "app_aiot/app_ota/*.c"
    "app_aiot/app_dynreg/*.c"
    "app_aiot/app_flash/*.c"
    )

idf_component_register(SRCS ${c_sdk_srcs}
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES mbedtls esp_https_ota lwip json app_update nvs_flash
                       )