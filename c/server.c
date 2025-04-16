#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "tlv_box.h"

// 定义日志输出宏
#define LOG(format,...) printf(format, ##__VA_ARGS__)

#define MAXLINE 1024 // 定义缓冲区大小
#define PORT 5035    // 定义服务器端口号

#define TEST_TYPE_0 0x0000
#define TEST_TYPE_1 0x0001
#define TEST_TYPE_2 0x0002
#define TEST_TYPE_3 0x0003
#define TEST_TYPE_4 0x0004
#define TEST_TYPE_5 0x0005
#define TEST_TYPE_6 0x0006
#define TEST_TYPE_7 0x0007
#define TEST_TYPE_8 0x0008
#define TEST_TYPE_9 0x0009

int main() {
    // 创建 UDP 套接字
    int serverDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverDescriptor < 0) {
        perror("Failed to create socket");
        return -1;
    }

    // 配置服务器地址
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    char recvMessage[MAXLINE]; // 用于接收客户端消息

    serverAddress.sin_family = AF_INET; // 使用 IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // 接收任意地址的消息
    serverAddress.sin_port = htons(PORT); // 设置端口号

    // 绑定套接字
    if (bind(serverDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to bind socket");
        return -1;
    }
    LOG("Server is running on port %d\n", PORT);

    while (1) {
        // 接收客户端消息
        ssize_t receivedBytes = recvfrom(serverDescriptor, recvMessage, MAXLINE, 0, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (receivedBytes < 0) {
            perror("Failed to receive data");
            continue;
        }
        LOG("Received %zd bytes from client\n", receivedBytes);

        // 解析接收到的 TLV 数据
        tlv_box_t *receivedBox = tlv_box_parse(recvMessage,receivedBytes);
        if (receivedBox == NULL) {
            printf("Failed to parse received TLV data\n");
            continue;
        }

        // 提取并显示多种类型数据
        {
            char receivedChar;
            if (tlv_box_get_char(receivedBox, TEST_TYPE_1, &receivedChar) == 0) {
                printf("Received (char): %c\n", receivedChar);
            }
        }
        {
            short receivedShort;
            if (tlv_box_get_short(receivedBox, TEST_TYPE_2, &receivedShort) == 0) {
                printf("Received (short): %d\n", receivedShort);
            }
        }
        {
            int receivedInt;
            if (tlv_box_get_int(receivedBox, TEST_TYPE_3, &receivedInt) == 0) {
                printf("Received (int): %d\n", receivedInt);
            }
        }
        {
            long receivedLong;
            if (tlv_box_get_long(receivedBox, TEST_TYPE_4, &receivedLong) == 0) {
                printf("Received (long): %ld\n", receivedLong);
            }
        }
        {
            float receivedFloat;
            if (tlv_box_get_float(receivedBox, TEST_TYPE_5, &receivedFloat) == 0) {
                printf("Received (float): %f\n", receivedFloat);
            }
        }
        {
            double receivedDouble;
            if (tlv_box_get_double(receivedBox, TEST_TYPE_6, &receivedDouble) == 0) {
                printf("Received (double): %f\n", receivedDouble);
            }
        }
        {
            char receivedString[128];
            int length = 128;
            if (tlv_box_get_string(receivedBox, TEST_TYPE_7, receivedString, &length) == 0) {
                printf("Received (string): %s\n", receivedString);
            }
        }
        {
            unsigned char receivedBytes[128];
            int length = 128;
            if (tlv_box_get_bytes(receivedBox, TEST_TYPE_8, receivedBytes, &length) == 0) {
                printf("Received (bytes): ");
                for (int i = 0; i < length; i++) {
                    printf("%02x ", receivedBytes[i]);
                }
                printf("\n");
                LOG("tlv_box_get_bytes success:  ");         
                int i = 0;
                for(i=0; i<length; i++) {
                    LOG("%d-", receivedBytes[i]); 
                } 
            }
        }
        // 回送消息给客户端
        ssize_t sentBytes = sendto(serverDescriptor, recvMessage, receivedBytes, 0, (struct sockaddr*)&clientAddress, clientAddressLength);
        if (sentBytes < 0) {
            perror("Failed to send data");
        } else {
            LOG("Echoed %zd bytes to client\n", sentBytes);
        }

        // 销毁接收到的 TLV box
        tlv_box_destroy(receivedBox);
    }

    close(serverDescriptor);
    return 0;
}