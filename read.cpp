#include "read.h"
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void msg(const char *message) { fprintf(stderr, "%s\n", message); }

void state_res(Conn *conn) {
  while (conn->wbuf_sent < conn->wbuf_size) {
    ssize_t rv = write(conn->fd, &conn->wbuf[conn->wbuf_sent],
                       conn->wbuf_size - conn->wbuf_sent);
    if (rv < 0 && errno == EINTR) {
      continue;
    }
    if (rv < 0 && errno == EAGAIN) {
      return; // 需要稍后再写
    }
    if (rv < 0) {
      perror("write() error");
      conn->state = STATE_END;
      return;
    }
    conn->wbuf_sent += (size_t)rv;
  }

  if (conn->wbuf_sent == conn->wbuf_size) {
    // 完成响应后，切换回请求状态
    conn->state = STATE_REQ;
    conn->wbuf_sent = 0;
    conn->wbuf_size = 0;
  }
}

bool try_one_request(Conn *conn) {
  if (conn->rbuf_size < 4) {
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
    return false;
  }

  printf("client says: %.*s\n", len, &conn->rbuf[4]);

  memcpy(&conn->wbuf[0], &len, 4);
  memcpy(&conn->wbuf[4], &conn->rbuf[4], len);
  conn->wbuf_size = 4 + len;

  size_t remain = conn->rbuf_size - 4 - len;
  if (remain) {
    memmove(conn->rbuf, &conn->rbuf[4 + len], remain);
  }
  conn->rbuf_size = remain;

  conn->state = STATE_RES;
  state_res(conn);

  return (conn->state == STATE_REQ);
}

bool try_fill_buffer(Conn *conn) {
  ssize_t rv = 0;
  do {
    size_t cap = sizeof(conn->rbuf) - conn->rbuf_size;
    rv = read(conn->fd, &conn->rbuf[conn->rbuf_size], cap);
  } while (rv < 0 && errno == EINTR);
  if (rv < 0 && errno == EAGAIN) {
    return false;
  }
  if (rv < 0) {
    perror("read() error");
    conn->state = STATE_END;
    return false;
  }
  if (rv == 0) {
    if (conn->rbuf_size > 0) {
      perror("unexpected EOF");
    } else {
      perror("EOF");
    }
    conn->state = STATE_END;
    return false;
  }

  conn->rbuf_size += (size_t)rv;
  while (try_one_request(conn)) {
  }
  return (conn->state == STATE_REQ);
}
