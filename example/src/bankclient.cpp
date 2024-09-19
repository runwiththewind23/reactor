#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int sockfd;  // Make sockfd global so it can be accessed in the signal handler.

// 发送报文，支持4字节的报头。
ssize_t tcpsend(int fd, void *data, size_t size) {
  char tmpbuf[1024];  // 临时的buffer，报文头部+报文内容。
  memset(tmpbuf, 0, sizeof(tmpbuf));
  memcpy(tmpbuf, &size, 4);        // 拼接报文头部。
  memcpy(tmpbuf + 4, data, size);  // 拼接报文内容。

  return send(fd, tmpbuf, size + 4, 0);  // 把请求报文发送给服务端。
}

// 接收报文，支持4字节的报头。
ssize_t tcprecv(int fd, void *data) {
  int len;
  recv(fd, &len, 4, 0);           // 先读取4字节的报文头部。
  return recv(fd, data, len, 0);  // 读取报文内容。
}

// Signal handler for SIGINT
void handle_sigint(int sig) {
  char buf[1024];
  printf("\nReceived SIGINT (Ctrl+C). Logging out...\n");

  // 注销业务。
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "<bizcode>00901</bizcode>");
  if (tcpsend(sockfd, buf, strlen(buf)) <= 0) {
    printf("tcpsend() failed during logout.\n");
  } else {
    printf("发送：%s\n", buf);

    memset(buf, 0, sizeof(buf));
    if (tcprecv(sockfd, buf) <= 0) {
      printf("tcprecv() failed during logout.\n");
    } else {
      printf("接收：%s\n", buf);
    }
  }

  close(sockfd);  // Close the socket before exiting.
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: ./bankclient ip port\n");
    printf("example: ./bankclient 10.0.16.13 5080\n\n");
    return -1;
  }

  struct sockaddr_in servaddr;
  char buf[1024];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket() failed.\n");
    return -1;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
    close(sockfd);
    return -1;
  }

  printf("connect ok.\n");

  // Register the SIGINT handler
  signal(SIGINT, handle_sigint);

  /////////////////////////////////////////
  // 登录业务。
  memset(buf, 0, sizeof(buf));
  sprintf(buf,
          "<bizcode>00101</bizcode><username>admin</username><passwd>123456</"
          "passwd>");
  if (tcpsend(sockfd, buf, strlen(buf)) <= 0) {
    printf("tcpsend() failed.\n");
    return -1;
  }
  printf("发送：%s\n", buf);

  memset(buf, 0, sizeof(buf));
  if (tcprecv(sockfd, buf) <= 0) {
    printf("tcprecv() failed.\n");
    return -1;
  }
  printf("接收：%s\n", buf);

  // Check if login was successful
  if (strstr(buf, "<retcode>0</retcode>") != NULL) {
    printf("Login successful.\n");

    //////////////////////////////////////////
    // 查询余额业务。
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "<bizcode>00201</bizcode><cardid>123456</cardid>");
    if (tcpsend(sockfd, buf, strlen(buf)) <= 0) {
      printf("tcpsend() failed.\n");
      return -1;
    }
    printf("发送：%s\n", buf);

    memset(buf, 0, sizeof(buf));
    if (tcprecv(sockfd, buf) <= 0) {
      printf("tcprecv() failed.\n");
      return -1;
    }
    printf("接收：%s\n", buf);
    //////////////////////////////////////////

  } else {
    printf("Login failed, cannot proceed further.\n");
  }

  //////////////////////////////////////////
  // Heartbeat.
  while (true) {
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "<bizcode>00001</bizcode>");
    if (tcpsend(sockfd, buf, strlen(buf)) <= 0) {
      printf("tcpsend() failed.\n");
      return -1;
    }
    printf("发送：%s\n", buf);

    memset(buf, 0, sizeof(buf));
    if (tcprecv(sockfd, buf) <= 0) {
      printf("tcprecv() failed.\n");
      return -1;
    }
    printf("接收：%s\n", buf);

    sleep(5);
  }
  //////////////////////////////////////////

  return 0;
}
