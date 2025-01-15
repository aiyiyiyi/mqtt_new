#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
​
// 定义MQTT报文类型
#define MQTT_CONNECT 0x10
#define MQTT_CONNACK 0x20
#define MQTT_PUBLISH 0x30
#define MQTT_PUBACK 0x40
#define MQTT_SUBSCRIBE 0x80
#define MQTT_SUBACK 0x90
#define MQTT_UNSUBSCRIBE 0xA0
#define MQTT_UNSUBACK 0xB0
#define MQTT_PINGREQ 0xC0
#define MQTT_PINGRESP 0xD0
#define MQTT_DISCONNECT 0xE0
​
// 定义MQTT连接标志
#define MQTT_CONNECT_FLAG_CLEAN 0x02
#define MQTT_CONNECT_FLAG_WILL 0x04
#define MQTT_CONNECT_FLAG_WILL_QOS0 0x00
#define MQTT_CONNECT_FLAG_WILL_QOS1 0x08
#define MQTT_CONNECT_FLAG_WILL_QOS2 0x10
#define MQTT_CONNECT_FLAG_WILL_RETAIN 0x20
#define MQTT_CONNECT_FLAG_PASSWORD 0x40
#define MQTT_CONNECT_FLAG_USERNAME 0x80
​
    // 定义MQTT报文结构体
    typedef struct mqtt_packet {
    unsigned char *data;
    unsigned int length;
} mqtt_packet_t;
​
    // 建立socket连接并返回socket文件描述符
    int
    socket_connect(char *address, int port) {
    struct sockaddr_in server_address;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Failed to create socket!\n");
        return -1;
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if ((inet_pton(AF_INET, address, &server_address.sin_addr)) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }
    if (connect(socket_fd, (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0) {
        printf("Connection Failed!\n");
        return -1;
    }
    return socket_fd;
}
// 打包MQTT连接报文
mqtt_packet_t *mqtt_connect(char *client_id, char *username, char *password) {
    mqtt_packet_t *packet = (mqtt_packet_t *)malloc(sizeof(mqtt_packet_t));
    unsigned char *data = (unsigned char *)malloc(256);
    unsigned int length = 0;
    // 固定报头
    data[length++] = MQTT_CONNECT;
    // 可变报头
    data[length++] = 0x0C;
    // 清理会话标志和协议版本号
    data[length++] = 'M';
    data[length++] = 'Q';
    data[length++] = 'T';
    data[length++] = 'T';
    data[length++] = 0x04;
    // 协议版本号 // 连接标志
    unsigned char flags = MQTT_CONNECT_FLAG_CLEAN;
    if (username != NULL) {
        flags |= MQTT_CONNECT_FLAG_USERNAME;
    }
    if (password != NULL) {
        flags |= MQTT_CONNECT_FLAG_PASSWORD;
    }
    data[length++] = flags;
    data[length++] = 0xFF;
    // 保持连接时间低8位
    data[length++] = 0xFF;
    // 保持连接时间高8位 // 剩余长度
    unsigned char remaining_length = length - 1;
    data[remaining_length++] = (unsigned char)(length - 2);
    packet->data = data;
    packet->length = length;
    return packet;
}
// 发送MQTT报文
void mqtt_send(int socket_fd, mqtt_packet_t *packet) {
    if (send(socket_fd, packet->data, packet->length, 0) < 0) {
        printf("Failed to send message!\n");
    }
}
// 接收MQTT报文
int mqtt_recv(int socket_fd, mqtt_packet_t *packet) {
    unsigned char header[2];
    if (recv(socket_fd, header, 2, 0) != 2) {
        printf("Failed to receive message header!\n");
        return -1
    }
    unsigned int remaining_length = 0;
    unsigned int multiplier = 1;
    int i = 1;
    do {
        if (recv(socket_fd, &header[i], 1, 0) != 1) {
            printf("Failed to receive remaining_length byte %d!\n", i);
            return -1;
        }
        remaining_length += (header[i] & 127) * multiplier;
        multiplier *= 128;
        i++;
    } while ((header[i - 1] & 128) != 0);
    packet->length = remaining_length + i;
    packet->data = (unsigned char *)malloc(packet->length);
    memcpy(packet->data, header, 2);
    if (recv(socket_fd, packet->data + 2, packet->length - 2, 0) !=
        packet->length - 2) {
        printf("Failed to receive full message!\n");
        return -1;
    }
    return 0;
}
​
​ int main(int argc, char *argv[]) {
    // 建立 TCP 连接
    int socket_fd = socket_connect("test.mosquitto.org", 1883);
    if (socket_fd == -1) {
        printf("Failed to connect to MQTT server!\n");
        return -1;
    }
    printf("Connected to MQTT server!\n");
    // 打包并发送 MQTT 连接报文
    mqtt_packet_t *connect_packet = mqtt_connect("test_client", NULL, NULL);
    mqtt_send(socket_fd, connect_packet);
    printf("Sent MQTT CONNECT packet!\n");
    free(connect_packet->data);
    free(connect_packet);
    // 接收 MQTT CONNACK 报文
    mqtt_packet_t *connack_packet =
        (mqtt_packet_t *)malloc(sizeof(mqtt_packet_t));
    if (mqtt_recv(socket_fd, connack_packet) != 0) {
        printf("Failed to receive MQTT CONNACK packet!\n");
        return -1;
    }
    if (connack_packet->data[1] != 0x00) {
        printf("MQTT server rejected connection!\n");
        return -1;
    }
    printf("Received MQTT CONNACK packet!\n");
    free(connack_packet->data);
    free(connack_packet);
    // 断开 TCP 连接 close(socket_fd); return 0;
}
