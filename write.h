// write.h
#ifndef WRITE_H
#define WRITE_H

#include "read.h" // 引入 Conn 结构体及常量声明
#include <iostream>
#include <sys/types.h>
#include <vector>

bool try_flush_buffer(Conn *conn);

#endif
