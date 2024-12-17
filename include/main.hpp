#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdint.h>

struct PrintTaskArgs {
public:
  const char *msg;
  uint64_t count;

  PrintTaskArgs() : msg(nullptr), count(0) {}
  PrintTaskArgs(const char *msg, uint64_t count) : msg(msg), count(count) {}
};

#endif // MAIN_HPP