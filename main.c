#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "leader.h"

void init_mqtt_send_data(MQTT_SendData *sendData);
void init_mqtt_recv_data(MQTT_RecvData *recvData);
int tcp_connect();
char mqtt_publish(char *message);
int mqtt_recv(unsigned char *Buff, unsigned int BuffLen);
int mqtt_send(unsigned char *buff, unsigned int buffLen);

int sockfd;

// 定义全局变量
MQTT_SendData mqttSendData;
MQTT_RecvData mqttRecvData;

// 初始化函数
void init_mqtt_send_data(MQTT_SendData *sendData) {
    memset(sendData->mqtt_Sdata, 0, MQTT_MAX_BUF);
    sendData->SdataLen = 0;
}

void init_mqtt_recv_data(MQTT_RecvData *recvData) {
    memset(recvData->mqtt_Rdata, 0, MQTT_MAX_BUF);
    recvData->RdataLen = 0;
}

int tcp_connect() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("sockfd创建失败");
        return -1;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    sin.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("tcp连接失败");
        close(sockfd);
        return -1;
    }
    printf("tcp连接成功\n");
    return 0;
}

int mqtt_connect() {
    unsigned char mqtt_Sdata[MQTT_MAX_BUF];
    unsigned int SdataLen = 0;
    int clientIDLen = strlen(ClientID);
    int userNameLen = strlen(Username);
    int passWordLen = strlen(Password);
    unsigned int overLength = 0;
    // 错误点二变量使用错误和剩余长度计算方法错误

    init_mqtt_send_data(&mqttSendData);
    init_mqtt_recv_data(&mqttRecvData);

    overLength = 10 + (clientIDLen + 2) + (userNameLen + 2) + (passWordLen + 2);
    mqtt_Sdata[SdataLen++] = CONNECT;

    do {
        unsigned char encodedByte = overLength % 128;
        overLength = overLength / 128;
        if (overLength > 0) {
            encodedByte |= 128;
        }
        mqtt_Sdata[SdataLen++] = encodedByte;
    } while (overLength > 0);

    mqtt_Sdata[SdataLen++] = 0;
    mqtt_Sdata[SdataLen++] = 4;
    mqtt_Sdata[SdataLen++] = 'M';
    mqtt_Sdata[SdataLen++] = 'Q';
    mqtt_Sdata[SdataLen++] = 'T';
    mqtt_Sdata[SdataLen++] = 'T';

    mqtt_Sdata[SdataLen++] = 4;
    // 错误点一:1100 0010  连接标志这是一个字节,不应该分成两个字节
    // mqtt_Sdata[SdataLen++] = 0xc2;
    // mqtt_Sdata[SdataLen++] = MQTT_CONNECT_CLEAN_SESSION;
    mqtt_Sdata[SdataLen++] = MQTT_CONNECT_USERNAME_FLAG |
                             MQTT_CONNECT_PASSWORD_FLAG |
                             MQTT_CONNECT_CLEAN_SESSION;
    mqtt_Sdata[SdataLen++] = 0;
    mqtt_Sdata[SdataLen++] = 100;

    mqtt_Sdata[SdataLen++] = (clientIDLen >> 8) & 0xFF;
    mqtt_Sdata[SdataLen++] = clientIDLen & 0xFF;
    memcpy(&mqtt_Sdata[SdataLen], ClientID, clientIDLen);
    SdataLen += clientIDLen;

    if (Username != NULL) {
        mqtt_Sdata[SdataLen++] = (userNameLen >> 8) & 0xFF;
        mqtt_Sdata[SdataLen++] = userNameLen & 0xFF;
        memcpy(&mqtt_Sdata[SdataLen], Username, userNameLen);
        SdataLen += userNameLen;
    }

    if (Password != NULL) {
        mqtt_Sdata[SdataLen++] = (passWordLen >> 8) & 0xFF;
        mqtt_Sdata[SdataLen++] = passWordLen & 0xFF;
        memcpy(&mqtt_Sdata[SdataLen], Password, passWordLen);
        SdataLen += passWordLen;
    }
    memcpy(mqttSendData.mqtt_Sdata, mqtt_Sdata, SdataLen);
    mqttSendData.SdataLen = SdataLen;

    if (mqtt_send(mqttSendData.mqtt_Sdata, mqttSendData.SdataLen) < 0) {
        printf("MQTT 数据发送失败\n");
        exit(1);
    } else {
        printf("MQTT 数据发送成功\n");

        int recv_len = mqtt_recv(mqttRecvData.mqtt_Rdata, MQTT_MAX_BUF);
        if (recv_len < 0) {
            printf("接收服务器响应失败\n");
            exit(1);
        } else {
            printf("接收服务器响应成功\n");
        }
    }
    return 0;
}
char mqtt_publish(char *message) {
    unsigned char mqtt_Sdata[MQTT_MAX_BUF];
    unsigned int SdataLen = 0;
    unsigned int overLenght = 0;
    unsigned int TopicLen = strlen(MESSAGE_REPORTING_TOPIC);
    unsigned int messageLen = strlen((const char *)message);

    init_mqtt_send_data(&mqttSendData);
    init_mqtt_recv_data(&mqttRecvData);

    overLenght = 10 + (TopicLen + 2) + 7 + messageLen;
    // 固定报头 0011 0000
    mqtt_Sdata[SdataLen++] = PUBLISH;
    do {
        unsigned char encodeByte = overLenght % 128;
        overLenght /= 128;
        if (overLenght > 0) {
            encodeByte |= 128;
        }
    } while (overLenght > 0);

    mqtt_Sdata[SdataLen++] = (TopicLen >> 8) & 0xFF;
    mqtt_Sdata[SdataLen++] = TopicLen & 0xFF;
    memcpy(&mqtt_Sdata[SdataLen], MESSAGE_REPORTING_TOPIC, TopicLen);
    SdataLen += TopicLen;

    mqtt_Sdata[SdataLen++] = (messageLen >> 8) & 0xFF;
    mqtt_Sdata[SdataLen++] = messageLen & 0xFF;
    memcpy(&mqtt_Sdata[SdataLen++], message, messageLen);

    memcpy(mqttSendData.mqtt_Sdata, mqtt_Sdata, SdataLen);
    mqttSendData.SdataLen = SdataLen;

    if (mqtt_send(mqttSendData.mqtt_Sdata, mqttSendData.SdataLen) < 0) {
        printf("消息发布失败\n");
        exit(1);
    }
    /*此处未完成 */
    return SdataLen;
}
int mqtt_subscribe() {
    init_mqtt_send_data(&mqttSendData);
    init_mqtt_recv_data(&mqttRecvData);

    unsigned char mqtt_Sdata[MQTT_MAX_BUF];
    unsigned int SdataLen = 0;
    unsigned int overLenght = 0;
    unsigned int TopicLen = strlen(MESSAGE_REPORTING_TOPIC);

    mqtt_Sdata[SdataLen++] = SUBSCRIBE;
    overLenght = 2 + (TopicLen + 2);
    do {
        unsigned char encodeByte = overLenght % 128;
        overLenght /= 128;
        if (overLenght > 0) {
            encodeByte |= 128;
        }
    } while (overLenght > 0);

    // matt_Sdata[SdataLen++] = ;  //

    // 主题过滤器
    mqtt_Sdata[SdataLen++] = (TopicLen >> 8) & 0xFF;
    mqtt_Sdata[SdataLen++] = TopicLen & 0xFF;
    memcpy(&mqtt_Sdata[SdataLen], MESSAGE_REPORTING_TOPIC, TopicLen);
    SdataLen += TopicLen;
    // 服务质量要求（qos）
    mqtt_Sdata[SdataLen++] = 0x01;

    memcpy(mqttSendData.mqtt_Sdata, mqtt_Sdata, SdataLen);
    mqttSendData.SdataLen = SdataLen;

    if (mqtt_send(mqttSendData.mqtt_Sdata, mqttSendData.SdataLen) < 0) {
        printf("MQTT订阅报文发送失败\n");
    } else {
        printf("MQTT订阅报文发送成功\n");
    }
    int recv_len = mqtt_recv(mqttRecvData.mqtt_Rdata, MQTT_MAX_BUF);
    if (recv_len <= 0) {
        printf("MQTT订阅接收返回消息失败\n");
    } else {
        printf("MQTT订阅接收返回消息成功\n");
    }

    return 0;
}

int mqtt_recv(unsigned char *Buff, unsigned int BuffLen) {
    int len = recv(sockfd, Buff, BuffLen, 0);
    if (len <= 0) {
        perror("recv失败");
        close(sockfd);
        return -1;
    } else {
        printf("接收到的数据长度: %d\n", len);
        printf("接收到的数据: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", Buff[i]);
        }
        printf("\n");
    }

    return len;
}

int mqtt_send(unsigned char *buff, unsigned int buffLen) {
    printf("发送数据长度: %u\n", buffLen);
    printf("发送的数据: ");
    for (unsigned int i = 0; i < buffLen; i++) {
        printf("%02X ", buff[i]);
    }
    printf("\n");

    if (send(sockfd, buff, buffLen, 0) < 0) {
        perror("send失败");
        close(sockfd);
        return -1;
    }
    return 0;
}

int main() {
    char message[MQTT_MAX_BUF];
    double TEMP = 20.0;

    printf("开始执行 main 函数\n");

    // 初始化结构体
    init_mqtt_send_data(&mqttSendData);
    init_mqtt_recv_data(&mqttRecvData);

    if (tcp_connect() < 0) {
        perror("TCP 连接失败\n");
        exit(1);
    }

    if (mqtt_connect() < 0) {
        printf("MQTT 连接失败\n");
        exit(1);
    }
    if (mqtt_subscribe() < 0) {
        printf("MQTT订阅失败");
        exit(1);
    } else {
        printf("MQTT订阅成功");
    }
    sprintf(message,
        "{\"services\": [{\"service_id\": "
        "\"stm32\",\"properties\":{\"TEMP\":%.1f}}]}",
        (double)(TEMP += 0.2));
    if (mqtt_publish(message) < 0) {
        printf("MQTT消息发布失败");
    } else {
        printf("MQTT消息发布成功");
    }

    close(sockfd);
    return 0;
}