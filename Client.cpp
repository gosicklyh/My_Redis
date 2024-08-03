#include <arpa/inet.h>
#include <assert.h>
#include <cstdint>
#include <cstdio>
#include <errno.h>
#include <iostream>
#include <netinet/ip.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static int32_t read_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buf, n);
    if (rv <= 0) {
      return -1; // error, or unexpected EOF
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buf, n);
    // std::cout << "\n```\n" << rv << "\n```\n";
    if (rv <= 0) {
      return -1; // error
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

const size_t k_max_msg = 4096;

static int32_t query(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    return -1;
  }

  char wbuf[4 + k_max_msg];
  memcpy(wbuf, &len, 4); // assume little endian
  // std::cout << wbuf[5]<<std::endl;


  memcpy(&wbuf[4], text, len);
  // printf("%s\n", wbuf);
  if (int32_t err = write_all(fd, wbuf, 4 + len)) {
    return err;
  }

  // 4 bytes header
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  int32_t err = read_full(fd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  memcpy(&len, rbuf, 4); // assume little endian
  if (len > k_max_msg) {
    msg("too long");
    return -1;
  }

  // reply body
  err = read_full(fd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  // do something
  rbuf[4 + len] = '\0';
  printf("server says: %s\n", &rbuf[4]);
  return 0;
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("connect");
  }

  // std::string tsm;
  // std::cin >> tsm;
  // int32_t err = query(fd, tsm.c_str());
  // multiple requests
  int32_t err = query(fd, "123");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "1234");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "12345");
  if (err) {
    goto L_DONE;
  }

L_DONE:
  close(fd);
  return 0;
}