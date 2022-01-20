# 文件说明

记录`Link SDK-4.x`的更新历史

# 更新内容

+ 2020-05-06: SDK 4.0.0版本正式发布
+ 2021-04-20: SDK 4.1.0版本正式发布
  +  新增安全隧道和远程调试功能
  +  新增AT模组驱动框架,支持模组快速适配
  +  mbedtls安全层抽象
  +  支持单报文多url的OTA
  +  新增基于mqtt的动态注册功能
  +  支持MQTT 5.0协议

# 模块状态


| 模块名                                      | 更新时间    | Commit ID
|---------------------------------------------|-------------|---------------------------------------------
| 核心模块(core)                              | xicai.cxc   | 5e86911557a6786321b62f676eb5b1137ad25c9a
| 基于MQTT的动态注册(components/dynreg-mqtt)  | 2021-10-14  | 3ec260703f401c925fbef6b6905de5eac4da4663
| 物模型模块(components/data-model)           | 2021-09-06  | 1d9270de816f7ff0f60c0b2a53d08ca4da8bab66
| 时间同步模块(components/ntp)                | 2021-09-16  | cb96f929c231ad8ee8c48dcf82167f3f6eb66dad
| 固件升级模块(components/ota)                | 2021-11-10  | 13f8cc22568375d132be9c3e1ad26a57f199f294
| 动态注册(components/dynreg)                 | 2020-07-30  | 190e3bed4080eeb07c4f9e907cb7c3d966dfab53



