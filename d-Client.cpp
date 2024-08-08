#include <cstddef>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket");
  }
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8989);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv < 0) {
    die("connect");
  }
  std::string msg;
  while (1) {
    std::cout << "Enter message (type 'end' to quit): ";
    std::getline(std::cin, msg);
    if (msg == "end") {
      break;
    }

    ssize_t n = write(fd, msg.c_str(), msg.size());
    if (n < 0) {
      die("wriet");
    }

    char rbuf[256];
    n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
      die("read");
    }
    std::cout << "Server says: " << rbuf << std::endl;
  }
  close(fd);
  return 0;
}