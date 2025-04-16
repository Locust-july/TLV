#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //UNIX标准函数
#include <sys/types.h>   //系统数据类型定义
#include <sys/socket.h>    //套接字相关函数和结构体
#include <netinet/in.h>  //互联网地址族

#define MAXLINE 1024
#define PORT 5035

int main(){
  //SOCK_DGRAM表示使用UDP协议
  int serverDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  
  // for storing  address of address
  socklen_t addressLength;
  
  //定义消息的长度
  char sendMessage[MAXLINE],recvMessage[MAXLINE];
  printf("\nEnter message :");
  //从标准输入读取用户输入的消息，存储在sendMessage缓冲区中。
  fgets(sendMessage,sizeof(sendMessage),stdin);
  
  // storing address in serverAddress
  struct sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddress.sin_port = htons(PORT);
  
  // 存储服务器地址
  addressLength = sizeof(serverAddress);

  // 建立连接，
  connect(serverDescriptor,(struct sockaddr*)&serverAddress,addressLength);
  
  //sendto：向服务器发送消息，参数--套接字描述符，发送缓冲区，缓冲区最大长度，标志位0表示默认行为
  //目标地址结构，地址结构长度
  sendto(serverDescriptor,sendMessage,MAXLINE,0,(struct sockaddr*)&serverAddress,addressLength);
  //recvfrom：从服务器接收消息，参数--套接字描述符，接收消息缓冲区，缓冲区最大长度，标志位
  //参数5和6: 这里为NULL，因为已经通过connect关联了地址，不需要再获取发送方信息
  recvfrom(serverDescriptor,recvMessage,MAXLINE,0,NULL,NULL);
 
  printf("\nServer's Echo : %s\n",recvMessage);

  return 0;
}