#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }
static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}
static void do_something(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read()error");
  }
  printf("client says: %s\n", rbuf);

  char wbuf[] = "world";
  write(connfd,wbuf, strlen(wbuf));
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

  rv = listen(fd, 128);
  if (rv) {
    die("listen()");
  }

  while (true) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd < 0) {
      continue; // error
    }

    do_something(connfd);
    close(connfd);
  }
  return 0;
}
