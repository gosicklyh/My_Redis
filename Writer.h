// Writer.h
#ifndef WRITER_H
#define WRITER_H

#include "Reader.h"
#include <cstdint>
#include <cstdio>


class Writer {
public:
  Writer();
  bool try_flush_buffer(int fd, uint32_t &state);
  uint8_t *get_wbuf();
  size_t &get_wbuf_size();

private:
  size_t wbuf_size;
  size_t wbuf_sent;
  uint8_t wbuf[4 + k_max_msg];
};

#endif // WRITER_H
