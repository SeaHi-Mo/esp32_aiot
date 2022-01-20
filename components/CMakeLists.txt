set(include_dirs LinkSDK/core LinkSDK/core/sysdep LinkSDK/core/utils 
app_aiot/data-model 
LinkSDK/components/data-model
LinkSDK/components/dynreg
LinkSDK/components/dynreg-mqtt
LinkSDK/components/ntp
LinkSDK/components/ota
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
    "app_aiot/data-model/*c"
    )

idf_component_register(SRCS ${c_sdk_srcs}
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES mbedtls)