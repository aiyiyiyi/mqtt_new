#ifndef LEADER_H
#define LEADER_H

#define MQTT_MAX_BUF 512

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

#define ClientID "67604637ef99673c8ad65ca8_stm32_0_1_2025011814"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password "f09152f9dd398d13a7edb26ae57a6d31cf474b81e4d7d3cb26aa8e92fc020f71"
//设备消息上报
#define MESSAGE_REPORTING_TOPIC "$oc/devices/67604637ef99673c8ad65ca8_stm32/sys/messages/up"

//消息
// #define MESSAGE 

// {
//     "services": [
//         {
//             "service_id": "stm32",
//             "properties": {
//                 "TEMP": 27
//             }
//         }
//     ]
// }

/* 摘自文档第14页 */
typedef enum {
    CONNECT = 0x10,
    CONNACK = 0x20,
    PUBLISH = 0x30,
    PUBACK = 0x40,
    PUBREC = 0x50,
    PUBREL = 0x60,
    PUBCOMP = 0x70,
    SUBSCRIBE = 0x82,
    SUBACK = 0x90,
    UNSUBSCRIBE = 0xA0,
    UNSUBACK = 0xB0,
    PINGREQ = 0xC0,
    PINGRESP = 0xD0,
    DISCONNECT = 0xE0
} message_type;

typedef enum {
    MQTT_CONNECT_CLEAN_SESSION = 0x20, /* 清理会话 */
    MQTT_CONNECT_WILL_FLAG = 0x04,     /* 遗嘱标志 */
    MQTT_CONNECT_WILL_QOS0 = 0x00,     /* 遗嘱QoS */
    MQTT_CONNECT_WILL_QOS1 = 0x08,
    MQTT_CONNECT_WILL_QOS2 = 0x10,
    MQTT_CONNECT_WILL_RETAIN = 0x20,   /* 遗嘱保留 */
    MQTT_CONNECT_USERNAME_FLAG = 0x80, /* 用户名 */
    MQTT_CONNECT_PASSWORD_FLAG = 0x40  /* 密码 */
} MQTT_connect_flag;

typedef struct {
    unsigned char mqtt_Sdata[MQTT_MAX_BUF];
    unsigned int SdataLen;
} MQTT_SendData;

typedef struct {
    unsigned char mqtt_Rdata[MQTT_MAX_BUF];
    unsigned int RdataLen;
} MQTT_RecvData;

extern MQTT_SendData mqttSendData;
extern MQTT_RecvData mqttRecvData;

#endif