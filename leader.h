#ifndef LEADER_H
#define LEADER_H

#define MQTT_MAX_BUF 512

#define SERVER_IP "117.78.5.125"
#define SERVER_PORT 1883

#define ClinetID "67604637ef99673c8ad65ca8_stm32_0_1_2025011312"
#define Username "67604637ef99673c8ad65ca8_stm32"
#define Password "7f5fc34c6dadf7c802a6559886f9f3c4b67ee45177f1ab86633f5e02c869b13d"

/*摘自文档第14页 */
typedef enum{
    CONNECT=0x10,
    CONNACK=0x20,
    PUBLISH=0x30,
    PUBACK=0x40,
    PUBREC=0x50,
    PUBREL=0x60,
    PUBCOMP=0x70,
    SUBSCRIBE=0x80,
    SUBACK=0x90,
    UNSUBSCRIBE=0xA0,
    UNSUBACK=0xB0,
    PINGREQ=0xC0,
    PINGRESP=0xD0,
    DISCONNECT=0xE0

}messsge_type;

typedef enum{ 
    MQTT_CONNECT_CLEAN_SESSION = 0X20,  /*清理会话*/
    MQTT_CONNECT_WELL_FLAGE=0x40,       /*遗嘱标志*/
    MQTT_CONNECT_WILL_QOS0=0x00,        /*遗嘱QoS*/ 
    MQTT_CONNECT_WILL_QOS1=0x01,
    MQTT_CONNECT_WILL_QOS2=0x02,
    MQTT_CONNECT_WILL_RETAIN=0x20,      /*遗嘱保留*/
    MQTT_CONNECT_NAME_FLAGE=0x80,        /*用户名*/
    MQTT_CONNECT_PASSWORD_FLAGE=0x40    /*密码*/
}MQTT_connectfliag;


typedef struct MQTT_SENDDATA{
   unsigned char mqtt_Sdata[MQTT_MAX_BUF]; 
    unsigned int SdataLen;
}MQTT_SendData;

typedef struct MQTT_RECVDATA{
   unsigned char mqtt_Rdata[MQTT_MAX_BUF]; 
    unsigned int RdataLen;  
}MQTT_RecvData;
#endif