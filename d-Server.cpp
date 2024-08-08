#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 定义一个结构体，用于存储套接字信息和线程信息
struct SockInfo {
  int fd;                  // 通信的文件描述符
  pthread_t tid;           // 线程ID
  struct sockaddr_in addr; // 客户端的地址信息
};

// 定义一个数组，存储多个客户端的信息
struct SockInfo infos[128];

// 子线程的工作函数，用于处理客户端的请求
void *working(void *arg) {
  while (1) {
    struct SockInfo *info = (struct SockInfo *)arg;
    // 接收数据
    char buf[1024];
    int ret = read(info->fd, buf, sizeof(buf));
    if (ret == 0) {
      // 客户端关闭连接
      printf("客户端已经关闭连接...\n");
      info->fd = -1;
      break;
    } else if (ret == -1) {
      // 接收数据失败
      printf("接收数据失败...\n");
      info->fd = -1;
      break;
    } else {
      // 将接收到的数据原样发送回客户端
      printf("客户端say: %s\n", buf);
      write(info->fd, buf, strlen(buf) + 1);
    }
  }
  return NULL;
}

int main() {
  // 1. 创建用于监听的套接字
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("socket");
    exit(0);
  }

  // 2. 绑定IP地址和端口号
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;   // 设置为IPv4协议
  addr.sin_port = htons(8989); // 设置端口号并转换为网络字节序
  addr.sin_addr.s_addr = INADDR_ANY; // 监听所有IP地址
  int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1) {
    perror("bind");
    exit(0);
  }

  // 3. 设置监听，最大连接队列为100
  ret = listen(fd, 100);
  if (ret == -1) {
    perror("listen");
    exit(0);
  }

  // 4. 等待并接受连接请求
  socklen_t len = sizeof(struct sockaddr);

  // 初始化数据结构体
  int max = sizeof(infos) / sizeof(infos[0]);
  for (int i = 0; i < max; ++i) {
    bzero(&infos[i], sizeof(infos[i])); // 清零结构体
    infos[i].fd = -1;                   // 设置文件描述符为无效
    infos[i].tid = -1;                  // 设置线程ID为无效
  }

  // 主线程负责监听，子线程负责通信
  while (1) {
    // 找到一个未使用的结构体槽位
    struct SockInfo *pinfo = NULL;
    for (int i = 0; i < max; ++i) {
      if (infos[i].fd == -1) {
        pinfo = &infos[i];
        break;
      }
      if (i == max - 1) {
        sleep(1); // 如果所有槽位都在使用，等待1秒再试
        i--;      // 重新检查
      }
    }

    // 接受客户端连接请求
    int connfd = accept(fd, (struct sockaddr *)&pinfo->addr, &len);
    printf("主线程，connfd: %d\n", connfd);
    if (connfd == -1) {
      perror("accept");
      exit(0);
    }
    pinfo->fd = connfd; // 保存客户端的文件描述符

    // 创建子线程处理客户端请求
    pthread_create(&pinfo->tid, NULL, working, pinfo);
    pthread_detach(pinfo->tid); // 设置子线程为分离状态
  }

  // 释放资源，关闭监听套接字
  close(fd);

  return 0;
}
