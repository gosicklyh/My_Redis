// Writer.cpp
#include "Writer.h"
#include <cassert>
#include <errno.h>
#include <unistd.h>

Writer::Writer() : wbuf_size(0), wbuf_sent(0) {}

bool Writer::try_flush_buffer(int fd, uint32_t &state) {
  ssize_t rv = 0;
  do {
    size_t remain = wbuf_size - wbuf_sent;
    rv = write(fd, &wbuf[wbuf_sent], remain);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    return false;
  }
  if (rv < 0) {
    perror("write() error");
    state = 2; // STATE_END
    return false;
  }
  wbuf_sent += (size_t)rv;
  assert(wbuf_sent <= wbuf_size);
  if (wbuf_sent == wbuf_size) {
    state = 0; // STATE_REQ
    wbuf_sent = 0;
    wbuf_size = 0;
    return false;
  }
  return true;
}

uint8_t *Writer::get_wbuf() { return wbuf; }

size_t &Writer::get_wbuf_size() { return wbuf_size; }
