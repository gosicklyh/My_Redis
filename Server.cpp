// Server.cpp
#include "Conn.h"
#include "Reader.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static void fd_set_nb(int fd) {
  errno = 0;
  int flags = fcntl(fd, F_GETFL, 0);
  if (errno) {
    die("fcntl error");
  }
  flags |= O_NONBLOCK;
  errno = 0;
  (void)fcntl(fd, F_SETFL, flags);
  if (errno) {
    die("fcntl error");
  }
}

//添加新通信fd到fd2conn
static int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd,
                               int epoll_fd) {
  struct sockaddr_in client_addr = {};
  socklen_t socklen = sizeof(client_addr);
  int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
  if (connfd < 0) {
    perror("accept() error");
    return -1;
  }

  fd_set_nb(connfd);

  Conn *conn = new Conn();
  if (!conn) {
    close(connfd);
    return -1;
  }
  conn->fd = connfd;

  if (fd2conn.size() <= (size_t)conn->fd) {
    fd2conn.resize(conn->fd + 1);
  }
  fd2conn[conn->fd] = conn;

  struct epoll_event event;
  event.data.fd = connfd;
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event) == -1) {
    perror("epoll_ctl EPOLL_CTL_ADD error");
    close(connfd);
    delete conn;
    return -1;
  }

  return 0;
}

//处理通信操作
static void connection_io(Conn *conn) {
  if (conn->state == 0) { // STATE_REQ
    while (true) {
      if (!conn->reader.try_fill_buffer(conn->fd)) {
        conn->state = 2; // STATE_END
        break;
      }

      if (!conn->reader.try_one_request(conn->fd, conn->writer.get_wbuf(),
                                        conn->writer.get_wbuf_size(),
                                        conn->state)) {
        break;
      }
    }
  } else if (conn->state == 1) { // STATE_RES
    while (true) {
      if (!conn->writer.try_flush_buffer(conn->fd, conn->state)) {
        break;
      }
    }
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

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0);
  int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  std::vector<Conn *> fd2conn;
  fd_set_nb(fd);

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    die("epoll_create1()");
  }

  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET; // 监听读事件，采用边缘触发模式
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
    perror("epoll_ctl EPOLL_CTL_ADD error");
    close(epoll_fd);
    close(fd);
    return -1;
  }
  std::vector<struct epoll_event> events(64); // 储存事件,初始容量为64

  while (true) {
    int nfds = epoll_wait(epoll_fd, events.data(), events.size(), -1);
    if (nfds == -1) {
      perror("epoll_wait()");
      break;
    }
    if (nfds == events.size()) {
      events.resize(events.size() * 2); // 如果返回事件数等于容量，则扩容
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == fd) {
        accept_new_conn(fd2conn, fd, epoll_fd);
      } else {
        Conn *conn = fd2conn[events[i].data.fd];
        if (conn) {
          connection_io(conn);
          if (conn->state == 2) { // STATE_END
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->fd, NULL);
            close(conn->fd);
            delete conn;
            fd2conn[conn->fd] = NULL;
          }
        }
      }
    }
  }

  close(fd);
  close(epoll_fd);
  return 0;
}
