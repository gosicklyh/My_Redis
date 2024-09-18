#include "do_someing.h"
#include "io_Read&Writer.h"
#include "main.h"
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// 将文件描述符设置为非阻塞模式
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

// 请求状态处理
static void state_req(Conn *conn) {
  while (try_fill_buffer(conn)) {
  }
}
// 响应状态处理
void state_res(Conn *conn) {
  while (try_flush_buffer(conn)) {
  }
}

// 连接对象存入 fd2conn 中
static void conn_put(std::vector<Conn *> &fd2conn, struct Conn *conn) {
  if (fd2conn.size() <= (size_t)conn->fd) {
    fd2conn.resize(conn->fd + 1);
  }
  fd2conn[conn->fd] = conn;
}

// 接受新的连接
static int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd) {
  // 接受连接
  struct sockaddr_in client_addr = {};
  socklen_t socklen = sizeof(client_addr);
  int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
  if (connfd < 0) {
    msg("accept() error");
    return -1;
  }

  // 将新的连接设置为非阻塞模式
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
  return 0;
}

// 处理连接的 I/O
static void connection_io(Conn *conn) {
  if (conn->state == STATE_REQ) {
    state_req(conn);
  } else if (conn->state == STATE_RES) {
    state_res(conn);
  } else {
    assert(0);
  }
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // 绑定地址和端口
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0); // 通配地址 0.0.0.0
  int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  // 监听
  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  // 存储所有客户端连接的映射，键为文件描述符
  std::vector<Conn *> fd2conn;

  // 将监听套接字设置为非阻塞模式
  fd_set_nb(fd);

  // 事件循环
  std::vector<struct pollfd> poll_args;
  while (true) {
    poll_args.clear();
    // 为方便起见，监听套接字放在第一个位置
    struct pollfd pfd = {fd, POLLIN, 0};
    poll_args.push_back(pfd);
    // 连接的文件描述符
    for (Conn *conn : fd2conn) {
      if (!conn) {
        continue;
      }
      struct pollfd pfd = {};
      pfd.fd = conn->fd;
      pfd.events = (conn->state == STATE_REQ) ? POLLIN : POLLOUT;
      pfd.events = pfd.events | POLLERR;
      poll_args.push_back(pfd);
    }

    // 轮询活动的文件描述符
    // 超时时间参数在这里无关紧要
    int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
    if (rv < 0) {
      die("poll");
    }

    // 处理活动的连接
    for (size_t i = 1; i < poll_args.size(); ++i) {
      if (poll_args[i].revents) {
        Conn *conn = fd2conn[poll_args[i].fd];
        connection_io(conn);
        if (conn->state == STATE_END) {
          // 客户端正常关闭，或发生错误
          // 销毁此连接
          fd2conn[conn->fd] = NULL;
          (void)close(conn->fd);
          free(conn);
        }
      }
    }

    // 如果监听套接字有活动，尝试接受新连接
    if (poll_args[0].revents) {
      (void)accept_new_conn(fd2conn, fd);
    }
  }

  return 0;
}
