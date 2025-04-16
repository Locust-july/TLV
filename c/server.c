#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include "tlv_box.h"

#define MAXLINE 1024 // 定义缓冲区大小
#define PORT 5035    // 定义服务器端口号

int main() {
    // 创建 UDP 套接字
    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    int number;
    socklen_t addressLength;
    char message[MAXLINE]; // 用于接收客户端消息

    // 配置服务器地址
    struct sockaddr_in serverAddress, clientAddress;
    serverAddress.sin_family = AF_INET; // 使用 IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // 接收来自任意地址的消息
    serverAddress.sin_port = htons(PORT); // 设置端口号

    // 绑定套接字到指定地址和端口
    bind(socketDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    printf("\nServer Started ...\n");

    while (1) {
        printf("\n");
        addressLength = sizeof(clientAddress);
        // 接收来自客户端的消息
        number = recvfrom(socketDescriptor, message, MAXLINE, 0, (struct sockaddr*)&clientAddress, &addressLength);

        if (number > 0) {
            printf("\nReceived TLV Data from Client\n");

            // 解析 TLV 数据
            tlv_box_t *parsedBox = tlv_box_parse((unsigned char*)message, number);
            if (parsedBox == NULL) {
                printf("Failed to parse TLV data\n");
                continue;
            }

            // 提取并打印 TLV 数据
            char value;
            if (tlv_box_get_char(parsedBox, 0x01, &value) == 0) {
                printf("Extracted char: %c\n", value); // 打印提取的字符
            }

            // 销毁解析后的 TLV box
            tlv_box_destroy(parsedBox);

            // 回送原始消息到客户端
            sendto(socketDescriptor, message, number, 0, (struct sockaddr*)&clientAddress, addressLength);
        }
    }
    return 0;
}