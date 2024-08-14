// Server.cpp
#include "Conn.h"
#include <arpa/inet.h>
#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
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

static void conn_put(std::vector<Conn *> &fd2conn, Conn *conn) {
  if (fd2conn.size() <= (size_t)conn->fd) {
    fd2conn.resize(conn->fd + 1);
  }
  fd2conn[conn->fd] = conn;
}

static int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd) {
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
  conn_put(fd2conn, conn);

  return 0;
}

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

  std::vector<struct pollfd> poll_args;
  while (true) {
    poll_args.clear();
    struct pollfd pfd = {fd, POLLIN, 0};
    poll_args.push_back(pfd);
    for (Conn *conn : fd2conn) {
      if (!conn) {
        continue;
      }
      struct pollfd pfd = {};
      pfd.fd = conn->fd;
      pfd.events =
          (conn->state == 0) ? POLLIN : POLLOUT; // STATE_REQ or STATE_RES
      pfd.events |= POLLERR;
      poll_args.push_back(pfd);
    }

    rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
    if (rv < 0) {
      die("poll");
    }

    for (size_t i = 1; i < poll_args.size(); ++i) {
      if (poll_args[i].revents) {
        Conn *conn = fd2conn[poll_args[i].fd];
        connection_io(conn);
        if (conn->state == 2) { // STATE_END
          fd2conn[conn->fd] = NULL;
          (void)close(conn->fd);
          delete conn;
        }
      }
    }

    if (poll_args[0].revents) {
      (void)accept_new_conn(fd2conn, fd);
    }
  }

  return 0;
}
