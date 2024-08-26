// write.cpp
#include "write.h"
#include <errno.h>
#include <iostream>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>

bool try_flush_buffer(Conn *conn) {
  ssize_t rv = 0;
  do {
    size_t remain = conn->wbuf_size - conn->wbuf_sent;
    rv = write(conn->fd, &conn->wbuf[conn->wbuf_sent], remain);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    return false;
  }
  if (rv < 0) {
    perror("write() error");
    conn->state = STATE_END;
    return false;
  }
  conn->wbuf_sent += (size_t)rv;
  if (conn->wbuf_sent == conn->wbuf_size) {
    conn->state = STATE_REQ;
    conn->wbuf_sent = 0;
    conn->wbuf_size = 0;
    return false;
  }
  return true;
}
