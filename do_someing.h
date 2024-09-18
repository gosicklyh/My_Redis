#ifndef IO_DOSOMEING_H
#define IO_DOSOMEING_H
#include <iostream>
#include <string>
#include <vector>
struct Conn;

bool try_one_request(Conn *conn);

void state_res(Conn *conn);

static bool cmd_is(const std::string &word, const char *cmd);

static uint32_t do_get(const std::vector<std::string> &cmd, uint8_t *res,
                       uint32_t *reslen);

static uint32_t do_set(const std::vector<std::string> &cmd, uint8_t *res,
                       uint32_t *reslen);

static uint32_t do_del(const std::vector<std::string> &cmd, uint8_t *res,
                       uint32_t *reslen);

static int32_t parse_req(const uint8_t *data, size_t len,
                         std::vector<std::string> &out);

static int32_t do_request(const uint8_t *req, uint32_t reqlen,
                          uint32_t *rescode, uint8_t *res, uint32_t *reslen);
#endif // IO_DOSOMEING_H