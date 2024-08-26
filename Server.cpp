#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

const size_t k_max_msg = 4096; // 最大消息长度

// 连接状态
enum {
  STATE_REQ = 0, // 处理请求
  STATE_RES = 1, // 发送响应
  STATE_END = 2, // 连接结束
};

// 连接结构体
struct Conn {
  int fd = -1;
  uint32_t state = 0; // 当前状态
  // 读缓冲区
  size_t rbuf_size = 0;
  uint8_t rbuf[4 + k_max_msg];
  // 写缓冲区
  size_t wbuf_size = 0;
  size_t wbuf_sent = 0;
  uint8_t wbuf[4 + k_max_msg];
};

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
    return -1; // 错误
  }

  // 设置新连接为非阻塞模式
  fd_set_nb(connfd);
  // 创建 Conn 结构体
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

static void state_req(Conn *conn);
static void state_res(Conn *conn);

static bool try_one_request(Conn *conn) {
  // 尝试从缓冲区中解析请求
  if (conn->rbuf_size < 4) {
    // 缓冲区数据不足，等待下次迭代
    return false;
  }
  uint32_t len = 0;
  memcpy(&len, &conn->rbuf[0], 4);
  if (len > k_max_msg) {
    msg("请求太长");
    conn->state = STATE_END;
    return false;
  }
  if (4 + len > conn->rbuf_size) {
    // 缓冲区数据不足，等待下次迭代
    return false;
  }

  // 收到一个完整的请求
  printf("client says: %.*s\n", len, &conn->rbuf[4]);

  // 生成回显响应
  memcpy(&conn->wbuf[0], &len, 4);
  memcpy(&conn->wbuf[4], &conn->rbuf[4], len);
  conn->wbuf_size = 4 + len;

  // 从缓冲区移除已处理的请求
  size_t remain = conn->rbuf_size - 4 - len;
  if (remain) {
    memmove(conn->rbuf, &conn->rbuf[4 + len], remain);
  }
  conn->rbuf_size = remain;

  // 切换状态到响应
  conn->state = STATE_RES;
  state_res(conn);

  // 如果请求已完全处理，继续外层循环
  return (conn->state == STATE_REQ);
}

static bool try_fill_buffer(Conn *conn) {
  // 尝试填充缓冲区
  assert(conn->rbuf_size < sizeof(conn->rbuf));
  ssize_t rv = 0;
  do {
    size_t cap = sizeof(conn->rbuf) - conn->rbuf_size;
    rv = read(conn->fd, &conn->rbuf[conn->rbuf_size], cap);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    // 读取到 EAGAIN，停止读取
    return false;
  }
  if (rv < 0) {
    msg("read() 错误");
    conn->state = STATE_END;
    return false;
  }
  if (rv == 0) {
    if (conn->rbuf_size > 0) {
      msg("意外的 EOF");
    } else {
      msg("EOF");
    }
    conn->state = STATE_END;
    return false;
  }

  conn->rbuf_size += (size_t)rv;
  assert(conn->rbuf_size <= sizeof(conn->rbuf));

  // 逐个处理请求
  while (try_one_request(conn)) {
  }
  return (conn->state == STATE_REQ);
}

static void state_req(Conn *conn) {
  while (try_fill_buffer(conn)) {
  }
}

static bool try_flush_buffer(Conn *conn) {
  ssize_t rv = 0;
  do {
    size_t remain = conn->wbuf_size - conn->wbuf_sent;
    rv = write(conn->fd, &conn->wbuf[conn->wbuf_sent], remain);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    // 读取到 EAGAIN，停止写入
    return false;
  }
  if (rv < 0) {
    msg("write() 错误");
    conn->state = STATE_END;
    return false;
  }
  conn->wbuf_sent += (size_t)rv;
  assert(conn->wbuf_sent <= conn->wbuf_size);
  if (conn->wbuf_sent == conn->wbuf_size) {
    // 响应已完全发送，切换回请求状态
    conn->state = STATE_REQ;
    conn->wbuf_sent = 0;
    conn->wbuf_size = 0;
    return false;
  }
  // 缓冲区中仍有数据，尝试继续写入
  return true;
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
        // 监听的 fd 有新连接
        while (accept_new_conn(fd2conn, fd, epoll_fd) == 0) {
        }
      } else {
        // 客户端 fd 有事件
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
