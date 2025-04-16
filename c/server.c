#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>  //系统数据类型定义
#include<sys/socket.h>  //套接字相关函数和结构体
#include<netinet/in.h>  //网络地址结构体和常量
#include<unistd.h>        //UNIX标准函数
// time

#define MAXLINE 1024     //最大消息场地
#define PORT 5035      //监听端口号

int main(){

  int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);//AF_INET表示IPv4协议，SOCK_DGRAM表示UDP协议
  int number;     //用于存储接受/发送的字节数
  socklen_t addressLength;   //客户端地址结构的长度
  char message[MAXLINE];   //用于存储消息的缓冲区

  struct sockaddr_in  serverAddress,clientAddress;
  serverAddress.sin_family = AF_INET;          //IPv4地址族
  serverAddress.sin_addr.s_addr=INADDR_ANY;  //监听所有网络接口
  serverAddress.sin_port=htons(PORT);        //设置端口号

  //将套接字与服务器地址结构绑定，使服务器能够在指定端口上接收数据。
  bind(socketDescriptor,(struct sockaddr*)&serverAddress,sizeof(serverAddress));

  printf("\nServer Started ...\n");

  while(1){
    printf("\n");
    //获取客户端地址结构的长度
    addressLength = sizeof(clientAddress);
    //接收客户端发送的数据
    //recvfrom函数用于接收数据，参数包括：套接字描述符、消息缓冲区、消息长度、标志、客户端地址结构和地址长度
    //返回值为接收到的字节数
    number = recvfrom(socketDescriptor,message,MAXLINE,0,(struct sockaddr*)&clientAddress,&addressLength);

    printf("\n Client's Message: %s ",message);
    //检查接受的字节数
    if(number<6)
      perror("send error");
    //将相同消息发送回客户端       
    sendto(socketDescriptor,message,number,0,(struct sockaddr*)&clientAddress,addressLength);
  }
  return 0;
}