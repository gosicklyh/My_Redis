#include "do_someing.h"
#include "main.h"
#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <vector>
// 键值存储的数据结构，使用 map 作为占位符
static std::map<std::string, std::string> g_map;

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
