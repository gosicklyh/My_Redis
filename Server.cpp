#include "io_Read&Writer.h"
#include "main.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <map>
#include <netinet/ip.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// 键值存储的数据结构，使用 map 作为占位符
static std::map<std::string, std::string> g_map;

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
static void state_res(Conn *conn) {
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

// 解析请求数据
static int32_t parse_req(const uint8_t *data, size_t len,
                         std::vector<std::string> &out) {
  if (len < 4) {
    return -1;
  }
  uint32_t n = 0;
  memcpy(&n, &data[0], 4);
  if (n > k_max_args) {
    return -1;
  }

  size_t pos = 4;
  while (n--) {
    if (pos + 4 > len) {
      return -1;
    }
    uint32_t sz = 0;
    memcpy(&sz, &data[pos], 4);
    if (pos + 4 + sz > len) {
      return -1;
    }
    out.push_back(std::string((char *)&data[pos + 4], sz));
    pos += 4 + sz;
  }

  if (pos != len) {
    return -1; // 有多余的数据
  }
  return 0;
}

// 处理 get 命令
static uint32_t do_get(const std::vector<std::string> &cmd, uint8_t *res,
                       uint32_t *reslen) {
  if (!g_map.count(cmd[1])) {
    return RES_NX; // key 不存在
  }
  std::string &val = g_map[cmd[1]];
  assert(val.size() <= k_max_msg);
  memcpy(res, val.data(), val.size());
  *reslen = (uint32_t)val.size();
  return RES_OK;
}

// 处理 set 命令
static uint32_t do_set(const std::vector<std::string> &cmd, uint8_t *res,
                       uint32_t *reslen) {
  (void)res;
  (void)reslen;
  g_map[cmd[1]] = cmd[2];
  return RES_OK;
}

// 处理 del 命令
static uint32_t do_del(const std::vector<std::string> &cmd, uint8_t *res,
                       uint32_t *reslen) {
  (void)res;
  (void)reslen;
  g_map.erase(cmd[1]);
  return RES_OK;
}

// 判断命令是否匹配
static bool cmd_is(const std::string &word, const char *cmd) {
  return 0 == strcasecmp(word.c_str(), cmd);
}

// 处理请求
static int32_t do_request(const uint8_t *req, uint32_t reqlen,
                          uint32_t *rescode, uint8_t *res, uint32_t *reslen) {
  std::vector<std::string> cmd;
  if (0 != parse_req(req, reqlen, cmd)) {
    msg("bad req");
    return -1;
  }
  if (cmd.size() == 2 && cmd_is(cmd[0], "get")) {
    *rescode = do_get(cmd, res, reslen);
  } else if (cmd.size() == 3 && cmd_is(cmd[0], "set")) {
    *rescode = do_set(cmd, res, reslen);
  } else if (cmd.size() == 2 && cmd_is(cmd[0], "del")) {
    *rescode = do_del(cmd, res, reslen);
  } else {
    // 未识别的命令
    *rescode = RES_ERR;
    const char *msg = "Unknown cmd";
    strcpy((char *)res, msg);
    *reslen = strlen(msg);
    return 0;
  }
  return 0;
}

// 尝试处理一个请求
bool try_one_request(Conn *conn) {
  // 尝试从缓冲区解析一个请求
  if (conn->rbuf_size < 4) {
    // 缓冲区数据不足，等待更多数据
    return false;
  }
  uint32_t len = 0;
  memcpy(&len, &conn->rbuf[0], 4);
  if (len > k_max_msg) {
    msg("too long");
    conn->state = STATE_END;
    return false;
  }
  if (4 + len > conn->rbuf_size) {
    // 数据不完整，等待更多数据
    return false;
  }

  // 获取一个完整的请求，生成响应
  uint32_t rescode = 0;
  uint32_t wlen = 0;
  int32_t err =
      do_request(&conn->rbuf[4], len, &rescode, &conn->wbuf[4 + 4], &wlen);
  if (err) {
    conn->state = STATE_END;
    return false;
  }
  wlen += 4;
  memcpy(&conn->wbuf[0], &wlen, 4);
  memcpy(&conn->wbuf[4], &rescode, 4);
  conn->wbuf_size = 4 + wlen;

  // 从读取缓冲区中移除已处理的请求
  // 注意：频繁的 memmove 操作效率低下，生产代码需优化
  size_t remain = conn->rbuf_size - 4 - len;
  if (remain) {
    memmove(conn->rbuf, &conn->rbuf[4 + len], remain);
  }
  conn->rbuf_size = remain;

  // 改变状态
  conn->state = STATE_RES;
  state_res(conn);

  // 如果请求已完全处理，继续外部循环
  return (conn->state == STATE_REQ);
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
