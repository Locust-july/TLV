#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "tlv_box.h"   // 引入 tlv_box.h 头文件

// 定义日志输出宏
#define LOG(format,...) printf(format, ##__VA_ARGS__)

#define MAXLINE 1024 // 定义缓冲区大小
#define PORT 5035    // 定义服务器端口号

// 定义测试用的类型常量
#define TEST_TYPE_0 0x00
#define TEST_TYPE_1 0x01
#define TEST_TYPE_2 0x02
#define TEST_TYPE_3 0x03
#define TEST_TYPE_4 0x04
#define TEST_TYPE_5 0x05
#define TEST_TYPE_6 0x06
#define TEST_TYPE_7 0x07
#define TEST_TYPE_8 0x08
#define TEST_TYPE_9 0x09

int main() {
    // 创建 UDP 套接字
    int serverDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    socklen_t addressLength;
    char recvMessage[MAXLINE]; // 用于接收服务器返回的消息

    // 配置服务器地址
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // 使用 IPv4
    serverAddress.sin_addr.s_addr = inet_addr("47.109.151.78"); // 服务器地址为本地
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
    unsigned char array[6] = {1, 2, 3, 4, 5, 6};
    tlv_box_put_bytes(box, TEST_TYPE_8, array, 6);   

    // 序列化 TLV 数据
    if (tlv_box_serialize(box) != 0) {
        printf("Failed to serialize TLV box\n");
        tlv_box_destroy(box); // 销毁 TLV box
        return -1;
    }
    //打印box创建成功消息
    LOG("boxes serialize success, %d bytes \n", tlv_box_get_size(box));

    tlv_box_t *boxes = tlv_box_create();  
    tlv_box_put_object(boxes, TEST_TYPE_9, box); // 将 box 作为对象添加到 boxes 中
    // 序列化嵌套的 boxes
    if (tlv_box_serialize(boxes) != 0) {
    LOG("boxes serialize failed !\n"); 
    return -1;
    }
    //打印 boxes创建成功的消息
    LOG("boxes serialize success, %d bytes \n", tlv_box_get_size(boxes));

    LOG("Serialized nested TLV data: ");
    unsigned char *nestedBuffer = tlv_box_get_buffer(boxes);
    size_t nestedSize = tlv_box_get_size(boxes);
    for (size_t i = 0; i < nestedSize; i++) {
        printf("%02x ", nestedBuffer[i]);
    }
    printf("\n");

    // 发送 TLV 数据到服务器
    ssize_t sentBytes = sendto(serverDescriptor, tlv_box_get_buffer(boxes), tlv_box_get_size(boxes), 0, (struct sockaddr*)&serverAddress, addressLength);
    if (sentBytes < 0) {
        perror("Failed to send data");
        return -1;
    }
    LOG("Sent %zd bytes to server\n", sentBytes);

    // 接收服务器回送的消息
    ssize_t receivedBytes = recvfrom(serverDescriptor, recvMessage, MAXLINE, 0, NULL, NULL);
    if (receivedBytes < 0) {
        perror("Failed to receive data");
        return -1;
    }
    LOG("Received %zd bytes from server\n", receivedBytes);

    // 解析接收到的 TLV 数据
    tlv_box_t *receivedBoxes = tlv_box_parse(recvMessage, receivedBytes); // 反序列化嵌套的 boxes
    if (receivedBoxes == NULL) {
        printf("Failed to parse received TLV data\n");
        return -1;
    }
/*=======================================剥离一层object===================================================*/
    // 提取嵌套的 box 对象
    tlv_box_t *receivedBox = NULL;
    if (tlv_box_get_object(receivedBoxes, TEST_TYPE_9, &receivedBox) != 0) {
        printf("Failed to extract nested box from received TLV data\n");
        tlv_box_destroy(receivedBoxes);
        return -1;
    }
/*=======================================================================================================*/
    // 提取多种类型数据
    {
        char receivedChar;
        if (tlv_box_get_char(receivedBox, TEST_TYPE_1, &receivedChar) == 0) {
            printf("\nServer's Echo (char): %c            Type:0x0001\n", receivedChar);
        }
    }
    {
        short receivedShort;
        if (tlv_box_get_short(receivedBox, TEST_TYPE_2, &receivedShort) == 0) {
            printf("Server's Echo (short): %d        Type:0x0002\n", receivedShort);
        }
    }
    {
        int receivedInt;
        if (tlv_box_get_int(receivedBox, TEST_TYPE_3, &receivedInt) == 0) {
            printf("Server's Echo (int): %d               Type:0x0003\n", receivedInt);
        }
    }
    {
        long receivedLong;
        if (tlv_box_get_long(receivedBox, TEST_TYPE_4, &receivedLong) == 0) {
            printf("Server's Echo (long): %ld             Type:0x0004\n", receivedLong);
        }
    }
    {
        float receivedFloat;
        if (tlv_box_get_float(receivedBox, TEST_TYPE_5, &receivedFloat) == 0) {
            printf("Server's Echo (float): %f             Type:0x0005\n", receivedFloat);
        }
    }
    {
        double receivedDouble;
        if (tlv_box_get_double(receivedBox, TEST_TYPE_6, &receivedDouble) == 0) {
            printf("Server's Echo (double): %f            Type:0x0006\n", receivedDouble);
        }
    }
    {
        char receivedString[128];
        int length = 128;
        if (tlv_box_get_string(receivedBox, TEST_TYPE_7, receivedString, &length) == 0) {
            printf("Server's Echo (string): %s            Type:0x0007\n", receivedString);
        }
    }
    {
        unsigned char receivedBytes[128];
        int length = 128;
        if (tlv_box_get_bytes(receivedBox, TEST_TYPE_8, receivedBytes, &length) == 0) {
            printf("Server's Echo (bytes): ");
            for (int i = 0; i < length; i++) {
                printf("%02x ", receivedBytes[i]);
            }
            printf("            Type:0x0008\n");
        }
    }

    // 销毁接收到的 TLV box
    tlv_box_destroy(receivedBox);
    tlv_box_destroy(receivedBoxes);

    // 销毁发送的 TLV box
    tlv_box_destroy(box);

    return 0;
}