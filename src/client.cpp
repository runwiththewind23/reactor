/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-07 17:14:25
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-16 13:45:26
 * @FilePath: /37/client.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-07 17:14:25
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-16 13:40:08
 * @FilePath: /37/client.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// 网络通讯的客户端程序。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
       if (argc != 3)
    {
        printf("usage:./client ip port\n"); 
        printf("example:./client 10.0.16.13 5080\n\n"); 
        return -1;
    }
    
    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];
 
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }
    
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
    }

    printf("connect ok.\n");
    printf("开始时间：%ld.\n",time(0));
    for (int ii=0;ii<100000;ii++){
        // 从命令行输入内容。
        memset(buf,0,sizeof(buf));
        sprintf(buf,"这是第%d个人。",ii); 

        char tmpbuf[1024];//临时的buffer，报文头部+报文内容
        memset(tmpbuf,0, sizeof(tmpbuf));
        int len = strlen(buf);//计算报文大小

        memcpy(tmpbuf, &len, 4);//拼接报文头部
        memcpy(tmpbuf+4,buf,len);//
        send(sockfd, tmpbuf,len +4, 0);
        recv(sockfd,&len,4,0);
        memset(buf,0,sizeof(buf));
        recv(sockfd,buf,len,0);
        //printf("recv:%s\n",buf);
    }
    printf("结束时间：%ld.\n",time(0));
 /* for (int ii=0;ii<10;ii++){
        // 从命令行输入内容。
        memset(buf,0,sizeof(buf));
        sprintf(buf,"这是第%d个人。",ii); 
        send(sockfd,buf,strlen(buf),0);
        memset(buf,0,sizeof(buf));
        recv(sockfd,buf,1024,0);
        printf("recv:%s\n",buf);
        sleep(1);
    }*/
    
} 