#include "read.h"
#include <cassert>
#include <cstdint>
#include <errno.h>
#include <map>
#include <netinet/ip.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
enum {
  RES_OK = 0,
  RES_ERR = 1,
  RES_NX = 2,
};

void msg(const char *message) { fprintf(stderr, "%s\n", message); }

bool cmd_is(const std::string &cmd, const std::string &expected) {
  return cmd == expected;
}

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
    msg("too loog");
    conn->state = STATE_END;
    return false;
  }
  if (4 + len > conn->rbuf_size) {
    return false;
  }
  // printf("client says: %.*s\n", len, &conn->rbuf[4]);

  uint32_t rescode = 0;
  uint32_t wlen = 0;
  int32_t err;

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
static std::map<std::string, std::string> g_map;

uint32_t do_get(const std::vector<std::string> &cmd, uint8_t *res,
                uint32_t *reslen) {
  if (!g_map.count(cmd[1])) {
    return RES_NX;
  }
  std::string &val = g_map[cmd[1]];
  assert(val.size() <= k_max_msg);
  memcpy(res, val.data(), val.size());
  *reslen = (uint32_t)val.size();
  return RES_OK;
}

uint32_t do_set(const std::vector<std::string> &cmd, uint8_t *res,
                uint32_t *reslen) {
  (void)res;
  (void)reslen;
  g_map[cmd[1]] = cmd[2];
  return RES_OK;
}

uint32_t do_del(const std::vector<std::string> &cmd, uint8_t *res,
                uint32_t *reslen) {
  (void)res;
  (void)reslen;
  g_map.erase(cmd[1]);
  return RES_OK;
}

int32_t parse_req(const uint8_t *data, size_t len,
                  std::vector<std::string> &out) {
  if (len < 4) {
    return -1;
  }
  uint32_t n = 0;
  memcpy(&n, &data[0], 4);
  if (n > k_max_msg) {
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
    return -1; // trailing garbage
  }
  return 0;
}

int32_t do_request(const uint8_t *req, uint32_t reqlen, uint32_t *rescode,
                   uint8_t *res, uint32_t *reslen) {
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
    // cmd is not recognized
    *rescode = RES_ERR;
    const char *msg = "Unknown cmd";
    strcpy((char *)res, msg);
    *reslen = strlen(msg);
    return 0;
  }
  return 0;
}

// The data structure for the key space. This is just a placeholder
// until we implement a hashtable in the next chapter.

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
