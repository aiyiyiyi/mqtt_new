#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leader.h"


int sockfd;

int tcp() {
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("sockfd创建失败");
        return -1;
    }
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    sin.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("tcp连接失败");
        close(sockfd);
        return -1;
    }
    printf("tcp连接成功");
    return 0;
}

int connect() {
    unsigned char mqtt_Sdata[MQTT_MAX_BUF];
    unsigned int SdataLen = 0;
    int clientIDLen = strlen(ClinetID);
    int userNameLen = strlen(Username);
    int passWordLen = strlen(Password);

    mqtt_Sdata[SdataLen++] = CONNECT;
    mqtt_Sdata[SdataLen++]=0;

    mqtt_Sdata[SdataLen++] = 0;
    mqtt_Sdata[SdataLen++] = 4;
    mqtt_Sdata[SdataLen++] = 'M';
    mqtt_Sdata[SdataLen++] = 'Q';
    mqtt_Sdata[SdataLen++] = 'T';
    mqtt_Sdata[SdataLen++] = 'T';
    mqtt_Sdata[SdataLen++] = 4;
    mqtt_Sdata[SdataLen++] = MQTT_CONNECT_CLEAN_SESSION;
    mqtt_Sdata[SdataLen++] = MQTT_CONNECT_NAME_FLAGE;
    mqtt_Sdata[SdataLen++] = MQTT_CONNECT_PASSWORD_FLAGE;
    mqtt_Sdata[SdataLen++] = 0;
    mqtt_Sdata[SdataLen++] = 100;

    mqtt_Sdata[SdataLen++] = (clientIDLen >> 8) & 0xFF;
    mqtt_Sdata[SdataLen++] = clientIDLen & 0xFF;
    memcpy(&mqtt_Sdata[SdataLen], ClinetID, clientIDLen);
    SdataLen += clientIDLen;

    if (Username != NULL) {
        mqtt_Sdata[SdataLen++] = (passWordLen >> 8) & 0xFF;
        mqtt_Sdata[SdataLen++] = passWordLen & 0xFF;
        memcpy(&mqtt_Sdata[SdataLen], Username, userNameLen);
        SdataLen += userNameLen;
    }

    if (Password != NULL) {
        mqtt_Sdata[SdataLen++] = (passWordLen >> 8) & 0xFF;
        mqtt_Sdata[SdataLen++] = passWordLen & 0xFF;
        memcpy(&mqtt_Sdata[SdataLen], Password, passWordLen);
        SdataLen += passWordLen;
    }
    mqtt_Sdata[1]=SdataLen-2;
    memcpy(mqttSendData.mqtt_Sdata, mqtt_Sdata, sizeof(mqtt_Sdata));
    mqttSendData.SdataLen = SdataLen;

    return 0;
}

int mqtt_recv(unsigned char *Buff,unsigned int BuffLen){
    if(recv(sockfd,&Buff,BuffLen,0)<0){
        perror("recv失败");
        close(sockfd);
        exit(1);
    }
    return 0;
}

int mqtt_send(unsigned char *buff, unsigned int buffLen) {
    if (send(sockfd, &buff, buffLen, 0) < 0) {
        perror("send失败");
        close(sockfd);
        exit(1);
    }
    return 0;
}

int main() {
    printf("开始执行 main 函数\n");
    if (tcp() < 0) {
        perror("TCP 连接失败\n");
        exit(1);
    }
    printf("TCP 连接成功\n");
    if (connect() < 0) {
        printf("MQTT 连接失败\n");
        exit(1);
    } else {
        mqtt_send(mqttSendData.mqtt_Sdata, mqttSendData.SdataLen);
       printf("MQTT 连接成功\n");
    }

    return 0;
}