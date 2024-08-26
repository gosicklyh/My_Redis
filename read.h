#ifndef READ_H
#define READ_H

#include <iostream>
#include <sys/types.h>

const size_t k_max_msg = 4096;

enum {
  STATE_REQ = 0,
  STATE_RES = 1,
  STATE_END = 2, // 标记连接以便删除
};

struct Conn {
  int fd = -1;
  uint32_t state = 0; // 状态: STATE_REQ 或 STATE_RES
  // 读缓冲区
  size_t rbuf_size = 0;
  uint8_t rbuf[4 + k_max_msg];
  // 写缓冲区
  size_t wbuf_size = 0;
  size_t wbuf_sent = 0;
  uint8_t wbuf[4 + k_max_msg];
};

// 函数声明
bool try_fill_buffer(Conn *conn);
bool try_one_request(Conn *conn);

#endif // READ_H
