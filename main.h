#ifndef IO_MAIN_H
#define IO_MAIN_H

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

// 定义全局变量和常量
const size_t k_max_msg = 4096;  // 最大消息长度
const size_t k_max_args = 1024; // 最大参数数量

enum {
  RES_OK = 0,  // 成功
  RES_ERR = 1, // 错误
  RES_NX = 2,  // 不存在
};

enum {
  STATE_REQ = 0, // 请求状态
  STATE_RES = 1, // 响应状态
  STATE_END = 2, // 连接结束，标记为需要删除
};

// 连接结构体
struct Conn {
  int fd = -1;
  uint32_t state = 0; // 连接状态，STATE_REQ 或 STATE_RES

  // 读缓冲区
  size_t rbuf_size = 0;
  uint8_t rbuf[4 + k_max_msg];

  // 写缓冲区
  size_t wbuf_size = 0;
  size_t wbuf_sent = 0;
  uint8_t wbuf[4 + k_max_msg];
};

// 输出消息的辅助函数
static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

#endif // IO_MAIN_H