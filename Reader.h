// Reader.h
#ifndef READER_H
#define READER_H

#include <cstdint>
#include <cstdio>

const size_t k_max_msg = 4096;

class Reader {
public:
  Reader();
  bool try_fill_buffer(int fd);
  bool try_one_request(int fd, uint8_t *wbuf, size_t &wbuf_size,
                       uint32_t &state);

private:
  size_t rbuf_size;
  uint8_t rbuf[4 + k_max_msg];
};

#endif // READER_H
