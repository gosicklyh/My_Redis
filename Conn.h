// Conn.h
#ifndef CONN_H
#define CONN_H

#include "Reader.h"
#include "Writer.h"

struct Conn {
  int fd = -1;
  uint32_t state = 0; // either STATE_REQ or STATE_RES
  Reader reader;
  Writer writer;
};

#endif // CONN_H
