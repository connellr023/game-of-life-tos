#include "../include/cell.hpp"
#include "../include/grid.hpp"
#include "../rpi3-drivers/include/framebuffer.hpp"
#include "../rpi3-drivers/include/uart0.hpp"
#include "../transient-os/include/kernel/kernel.hpp"
#include "../transient-os/include/utils/concurrency/atomic.hpp"

using namespace kernel::threads;

void cell_thread(void *arg) {
  CellThreadArg *cell = reinterpret_cast<CellThreadArg *>(arg);

  while (true) {
    {
      AtomicBlock guard;
      cell->render();
    }
  }
}

int main() {
  uart0::init();
  kernel::set_output_handler(&uart0::puts);

  if (!framebuffer::init()) {
    uart0::puts("Framebuffer initialization failed\n");
    return 1;
  }

  framebuffer::fill_screen(0xFF0000);
  uart0::puts("Framebuffer initialized\n");

  // Game initialization
  CellGrid grid_a = CellGrid();
  CellGrid grid_b = CellGrid();

  uart0::puts("Cell grids initialized\n");

  CellThreadArg args[GRID_ROWS][GRID_COLS] = {{CellThreadArg()}};
  ThreadControlBlock threads[GRID_ROWS][GRID_COLS] = {{ThreadControlBlock()}};

  uart0::puts("Declared arrays\n");

  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      threads[i][j] = ThreadControlBlock(&cell_thread, 2000, &args[i][j]);

      args[i][j].current_grid = &grid_a;
      args[i][j].next_grid = &grid_b;
      args[i][j].x = i;
      args[i][j].y = j;

      uart0::puts("Thread initialized\n");
    }
  }

  // Schedule each thread
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      if (!kernel::schedule_thread(&threads[i][j])) {
        uart0::puts("Failed to schedule thread\n");
        return 1;
      }
    }
  }

  uart0::puts("Starting kernel\n");
  kernel::start();

  return 0;
}