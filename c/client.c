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

int main() {
    // 创建 UDP 套接字
    int serverDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    socklen_t addressLength;
    char recvMessage[MAXLINE]; // 用于接收服务器返回的消息

    // 配置服务器地址
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // 使用 IPv4
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器地址为本地
    serverAddress.sin_port = htons(PORT); // 设置端口号

    addressLength = sizeof(serverAddress);

    // 创建 TLV 数据
    tlv_box_t *box = tlv_box_create(); // 初始化 TLV box
    tlv_box_put_char(box, 0x01, 'A'); // 添加一个字符类型数据，标识符为 0x01，值为 'A'

    // 序列化 TLV 数据
    if (tlv_box_serialize(box) != 0) {
        printf("Failed to serialize TLV box\n");
        tlv_box_destroy(box); // 销毁 TLV box
        return -1;
    }
    LOG("boxes serialize success, %d bytes \n", tlv_box_get_size(box));

    // 发送 TLV 数据到服务器
    ssize_t sentBytes = sendto(serverDescriptor, tlv_box_get_buffer(box), tlv_box_get_size(box), 0, (struct sockaddr*)&serverAddress, addressLength);
    if (sentBytes < 0) {
        perror("Failed to send data");
        tlv_box_destroy(box); // 销毁 TLV box
        return -1;
    }
    LOG("Sent %zd bytes to server\n", sentBytes);

    // 接收服务器回送的消息
    ssize_t receivedBytes = recvfrom(serverDescriptor, recvMessage, MAXLINE, 0, NULL, NULL);
    if (receivedBytes < 0) {
        perror("Failed to receive data");
        tlv_box_destroy(box); // 销毁 TLV box
        return -1;
    }
    LOG("Received %zd bytes from server\n", receivedBytes);

    // 打印接收到的原始字节内容
    LOG("Raw data received: ");
    for (ssize_t i = 0; i < receivedBytes; i++) {
        printf("%02x ", (unsigned char)recvMessage[i]);
    }
    printf("\n");

    // 解析接收到的 TLV 数据
    tlv_box_t *receivedBox = tlv_box_parse(recvMessage, receivedBytes);
    if (receivedBox == NULL) {
        printf("Failed to parse received TLV data\n");
        tlv_box_destroy(box); // 销毁 TLV box
        return -1;
    }

    // 提取字符类型数据
    char receivedChar;
    if (tlv_box_get_char(receivedBox, 0x01, &receivedChar) == 0) {
        printf("\nServer's Echo: %c\n", receivedChar); // 打印服务器回送的字符
    } else {
        printf("\nFailed to extract character from received TLV data\n");
    }

    // 销毁接收到的 TLV box
    tlv_box_destroy(receivedBox);

    // 销毁发送的 TLV box
    tlv_box_destroy(box);

    return 0;
}