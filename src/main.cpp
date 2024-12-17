#include "../lib-rpi3-drivers/include/uart0.hpp"

int main() {
  uart0::init();
  uart0::puts("Hello, this is main.cpp in game-of-life-tos!\n");

  return 0;
}