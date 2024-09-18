#ifndef IO_READ_H
#define IO_READ_H

#include <cstdint>
#include <vector>

struct Conn;

bool try_fill_buffer(Conn *conn);
bool try_one_request(Conn *conn);
bool try_flush_buffer(Conn *conn);

#endif // IO_READ_H
