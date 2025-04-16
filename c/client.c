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

// 定义测试用的类型常量
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
    socklen_t addressLength;
    char recvMessage[MAXLINE]; // 用于接收服务器返回的消息

    // 配置服务器地址
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // 使用 IPv4
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器地址为本地
    serverAddress.sin_port = htons(PORT); // 设置端口号

    addressLength = sizeof(serverAddress);
    // 连接到服务器  (可有可无?)
    connect(serverDescriptor,(struct sockaddr*)&serverAddress,addressLength);
    // 创建 TLV 数据
    tlv_box_t *box = tlv_box_create(); // 初始化 TLV box
  // 向 box 中添加各种类型的数据
  tlv_box_put_char(box, TEST_TYPE_1, 'x'); // 添加字符
  tlv_box_put_short(box, TEST_TYPE_2, (short)2); // 添加短整型
  tlv_box_put_int(box, TEST_TYPE_3, (int)3); // 添加整型
  tlv_box_put_long(box, TEST_TYPE_4, (long)4); // 添加长整型
  tlv_box_put_float(box, TEST_TYPE_5, (float)5.67); // 添加浮点型
  tlv_box_put_double(box, TEST_TYPE_6, (double)8.91); // 添加双精度浮点型
  tlv_box_put_string(box, TEST_TYPE_7, (char *)"hello world !"); // 添加字符串

    // 添加字节数组
    unsigned char array[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    tlv_box_put_bytes(box, TEST_TYPE_9, array, 4);

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

    // 解析接收到的 TLV 数据                //recevMessage为接收消息，received Bytes为字节
    tlv_box_t *receivedBox = tlv_box_parse(recvMessage, receivedBytes);   //反序列化
    if (receivedBox == NULL) {
        printf("Failed to parse received TLV data\n");
        tlv_box_destroy(box); // 销毁 TLV box
        return -1;
    }

    // 提取多种类型数据
    {
        char receivedChar;
        if (tlv_box_get_char(receivedBox, 0x0001, &receivedChar) == 0) {
            printf("\nServer's Echo (char): %c            Type:0x0001\n", receivedChar);
        }
    }
    {
        short receivedShort;
        if (tlv_box_get_short(receivedBox, 0x0002, &receivedShort) == 0) {
            printf("Server's Echo (short): %d        Type:0x0002\n", receivedShort);
        }
    }
    {
        int receivedInt;
        if (tlv_box_get_int(receivedBox, 0x0003, &receivedInt) == 0) {
            printf("Server's Echo (int): %d               Type:0x0003\n", receivedInt);
        }
    }
    {
        long receivedLong;
        if (tlv_box_get_long(receivedBox, 0x0004, &receivedLong) == 0) {
            printf("Server's Echo (long): %ld             Type:0x0004\n", receivedLong);
        }
    }
    {
        float receivedFloat;
        if (tlv_box_get_float(receivedBox, 0x0005, &receivedFloat) == 0) {
            printf("Server's Echo (float): %f             Type:0x0005\n", receivedFloat);
        }
    }
    {
        double receivedDouble;
        if (tlv_box_get_double(receivedBox, 0x0006, &receivedDouble) == 0) {
            printf("Server's Echo (double): %f            Type:0x0006\n", receivedDouble);
        }
    }
    {
        char receivedString[128];
        int length = 128;
        if (tlv_box_get_string(receivedBox, 0x0007, receivedString, &length) == 0) {
            printf("Server's Echo (string): %s            Type:0x0007\n", receivedString);
        }
    }
    {
        unsigned char receivedBytes[128];
        int length = 128;
        if (tlv_box_get_bytes(receivedBox, 0x0008, receivedBytes, &length) == 0) {
            printf("Server's Echo (bytes): ");
            for (int i = 0; i < length; i++) {
                printf("%02x ", receivedBytes[i]);
            }
            printf("            Type:0x0008\n");
        }
    }

    // 销毁接收到的 TLV box
    tlv_box_destroy(receivedBox);

    // 销毁发送的 TLV box
    tlv_box_destroy(box);

    return 0;
}