// Server.cpp
#include "read.h"
#include "write.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/ip.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// 打印信息
static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

// 打印错误信息并退出
static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

// 设置文件描述符为非阻塞模式
static void fd_set_nb(int fd) {
  errno = 0;
  int flags = fcntl(fd, F_GETFL, 0);
  if (errno) {
    die("fcntl error");
    return;
  }

  flags |= O_NONBLOCK;

  errno = 0;
  (void)fcntl(fd, F_SETFL, flags);
  if (errno) {
    die("fcntl error");
  }
}

// 将连接保存到 fd2conn 映射中
static void conn_put(std::vector<Conn *> &fd2conn, struct Conn *conn) {
  if (fd2conn.size() <= (size_t)conn->fd) {
    fd2conn.resize(conn->fd + 1);
  }
  fd2conn[conn->fd] = conn;
}

// 接受新连接
static int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd,
                               int epoll_fd) {
  // 接受连接
  struct sockaddr_in client_addr = {};
  socklen_t socklen = sizeof(client_addr);
  int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
  if (connfd < 0) {
    msg("accept() error");
    return -1; 
  }
  std::cout << "建立新链接: " << connfd << std::endl;
  // 设置新连接为非阻塞模式
  fd_set_nb(connfd);

  struct Conn *conn = (struct Conn *)malloc(sizeof(struct Conn));
  if (!conn) {
    close(connfd);
    return -1;
  }
  conn->fd = connfd;
  conn->state = STATE_REQ;
  conn->rbuf_size = 0;
  conn->wbuf_size = 0;
  conn->wbuf_sent = 0;
  conn_put(fd2conn, conn);

  // 将新连接加入 epoll 监听
  struct epoll_event event;
  event.data.fd = connfd;
  event.events = EPOLLIN | EPOLLET; // 监听读事件，边缘触发模式
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event) < 0) {
    msg("epoll_ctl error");
    close(connfd);
    free(conn);
    return -1;
  }

  return 0;
}
static void state_req(Conn *conn) {
  while (try_fill_buffer(conn)) {
  }
}

static void state_res(Conn *conn) {
  while (try_flush_buffer(conn)) {
  }
}

static void connection_io(Conn *conn) {
  if (conn->state == STATE_REQ) {
    state_req(conn);
  } else if (conn->state == STATE_RES) {
    state_res(conn);
  } else {
    assert(0); // 不应到达这里
  }
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // 绑定地址
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0); // 通配地址 0.0.0.0
  int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  // 监听连接
  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  // 保存所有客户端连接，使用文件描述符作为索引
  std::vector<Conn *> fd2conn;

  // 设置监听的文件描述符为非阻塞模式
  fd_set_nb(fd);

  // 创建 epoll 实例
  int epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    die("epoll_create1()");
  }

  // 将监听的 fd 加入 epoll 监听
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET; // 监听读事件，边缘触发模式
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
    die("epoll_ctl()");
  }

  // 事件循环
  while (true) {
    struct epoll_event events[1024];
    int n = epoll_wait(epoll_fd, events, 1024, 1000); // 1 秒超时
    if (n < 0) {
      die("epoll_wait()");
    }

    for (int i = 0; i < n; ++i) {
      if (events[i].data.fd == fd) {
        // 建立新链接
        accept_new_conn(fd2conn, fd, epoll_fd);
      } else {
        // 处理客户端fd事件
        Conn *conn = fd2conn[events[i].data.fd];
        if (conn) {
          connection_io(conn);
          if (conn->state == STATE_END) {
            // 连接结束，清理资源
            fd2conn[conn->fd] = NULL;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->fd, NULL);
            close(conn->fd);
            free(conn);
          }
        }
      }
    }
  }

  return 0;
}
