#include "../include/main.hpp"
#include "../lib-rpi3-drivers/include/uart0.hpp"
#include "../transient-os/include/kernel/kernel.hpp"
#include "../transient-os/include/utils/concurrency/atomic.hpp"

using namespace kernel::threads;

void print_task(void *arg) {
  PrintTaskArgs *args = reinterpret_cast<PrintTaskArgs *>(arg);

  // Print thread ID
  {
    AtomicBlock block;
    uart0::hex(kernel::get_thread_id());
    uart0::puts(": ");
    uart0::puts("Starting thread\n");
  }

  asm volatile("wfi");

  for (uint64_t i = 0; i < args->count; i++) {
    {
      AtomicBlock block;
      uart0::hex(kernel::get_thread_id());
      uart0::puts(": ");
      uart0::puts(args->msg);
    }
  }

  {
    AtomicBlock block;
    uart0::hex(kernel::get_thread_id());
    uart0::puts(": completed!\n");
  }
}

int main() {
  uart0::init();
  kernel::init_dbg_out(&uart0::puts, &uart0::hex);

  uart0::puts("Hello, world!\n");

  // Create and schedule n tasks
  constexpr uint64_t n = 6;
  ThreadControlBlock handles[n];

  PrintTaskArgs handle_args("Thread!\n", 3);

  for (uint64_t i = 0; i < n; i++) {
    handles[i] = ThreadControlBlock(&print_task, 2000, &handle_args);

    if (!kernel::schedule_thread(handles + i)) {
      uart0::puts("Failed to schedule thread\n");
      break;
    }
  }

  kernel::start();

  return 0;
}