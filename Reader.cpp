// Reader.cpp
#include "Reader.h"
#include <cassert>
#include <cstring>
#include <errno.h>
#include <unistd.h>

Reader::Reader() : rbuf_size(0) {}

bool Reader::try_fill_buffer(int fd) {
  assert(rbuf_size < sizeof(rbuf));
  ssize_t rv = 0;
  do {
    size_t cap = sizeof(rbuf) - rbuf_size;
    rv = read(fd, &rbuf[rbuf_size], cap);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    return false;
  }
  if (rv < 0) {
    perror("read() error");
    return false;
  }
  if (rv == 0) {
    if (rbuf_size > 0) {
      printf("unexpected EOF\n");
    } else {
      printf("EOF\n");
    }
    return false;
  }

  rbuf_size += (size_t)rv;
  assert(rbuf_size <= sizeof(rbuf));
  return true;
}

bool Reader::try_one_request(int fd, uint8_t *wbuf, size_t &wbuf_size,
                             uint32_t &state) {
  if (rbuf_size < 4) {
    return false;
  }
  uint32_t len = 0;
  memcpy(&len, &rbuf[0], 4);
  if (len > k_max_msg) {
    printf("too long\n");
    state = 2; // STATE_END
    return false;
  }
  if (4 + len > rbuf_size) {
    return false;
  }

  printf("client says: %.*s\n", len, &rbuf[4]);

  memcpy(&wbuf[0], &len, 4);
  memcpy(&wbuf[4], &rbuf[4], len);
  wbuf_size = 4 + len;

  size_t remain = rbuf_size - 4 - len;
  if (remain) {
    memmove(rbuf, &rbuf[4 + len], remain);
  }
  rbuf_size = remain;

  state = 1;           // STATE_RES
  return (state == 0); // STATE_REQ
}
