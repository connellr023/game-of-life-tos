#include "../include/grid.hpp"
#include "../rpi3-drivers/include/clock.hpp"
#include "../rpi3-drivers/include/framebuffer.hpp"
#include "../rpi3-drivers/include/uart0.hpp"
#include "../transient-os/include/kernel/kernel.hpp"
#include "../transient-os/include/kernel/sys/sys_calls.hpp"
#include "../transient-os/include/utils/concurrency/atomic.hpp"

/**
 * ### Grid swap thread
 * @brief This thread is responsible for swapping the current and next grids
 * once all the cell threads have finished updating the next grid.
 */
void grid_swap_thread(void *arg) {
  GridManager *grid_manager = reinterpret_cast<GridManager *>(arg);

  while (true) {
    if (grid_manager->get_ready_threads() >= CELL_COUNT) {
      AtomicBlock guard;
      grid_manager->swap_grids();
    }

    // Print message and effectively pass control to the next thread (since this
    // is a system call)
    kernel::sys::puts("Grid swap thread done!\n");
  }
}

/**
 * ### Cell thread
 * @brief This thread is responsible for updating the state of a single cell
 * in the grid.
 */
void cell_thread(void *arg) {
  CellThreadArg *cell = reinterpret_cast<CellThreadArg *>(arg);

  while (true) {
    // Render the cell
    cell->render();

    // Check if the cell is alive
    CellState current_state;

    // Safe get the current state
    {
      AtomicBlock guard;
      current_state = cell->get_current_state();
    }

    // Count the number of alive neighbors
    int alive_neighbors = 0;

    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        if (i == 0 && j == 0) {
          continue;
        }

        const int x = cell->get_x() + i;
        const int y = cell->get_y() + j;

        if (x < 0 || x >= GRID_ROWS || y < 0 || y >= GRID_COLS) {
          continue;
        }

        CellState state;

        {
          AtomicBlock guard;
          state = cell->get_grid_manager()->get_current_grid()->get_cell(x, y);
        }

        if (state == CellState::Alive) {
          alive_neighbors++;
        }
      }
    }

    // Update the cell state
    CellState next_state;

    if (current_state == CellState::Alive) {
      if (alive_neighbors < 2 || alive_neighbors > 3) {
        next_state = CellState::Dead;
      } else {
        next_state = CellState::Alive;
      }
    } else {
      if (alive_neighbors == 3) {
        next_state = CellState::Alive;
      } else {
        next_state = CellState::Dead;
      }
    }

    // Safe update the next grid
    {
      AtomicBlock guard;
      cell->get_grid_manager()->get_next_grid()->set_cell(
          cell->get_x(), cell->get_y(), next_state);
    }

    // Safe increment the number of ready threads
    {
      AtomicBlock guard;
      cell->get_grid_manager()->increment_ready_threads();
    }

    // Pass control to the next thread
    kernel::sys::yield();
  }
}

int main() {
  uart0::init();
  kernel::set_output_handler(&uart0::puts);

  if (!framebuffer::init()) {
    uart0::puts("Framebuffer initialization failed\n");
    return 1;
  }

  framebuffer::fill_screen(CELL_DEAD_COLOR);
  uart0::puts("Framebuffer initialized\n");

  // Game initialization
  CellGrid grid_a;
  CellGrid grid_b;

  // Randomly initialize the grid
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      const uint64_t seed = clock::current_micros();
      grid_a.set_cell(i, j, seed % 3 == 0 ? CellState::Alive : CellState::Dead);
    }
  }

  CellThreadArg args[GRID_ROWS][GRID_COLS];
  ThreadControlBlock threads[GRID_ROWS][GRID_COLS];

  GridManager grid_manager;
  grid_manager.init(&grid_a, &grid_b);

  ThreadControlBlock grid_swap_thread_tcb;
  grid_swap_thread_tcb.init(&grid_swap_thread, 1000, &grid_manager);

  uart0::puts("Declared arrays\n");

  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      const uint64_t burst_time = clock::random_range(1900, 2500);

      threads[i][j].init(&cell_thread, burst_time, &args[i][j]);
      args[i][j].init(&grid_manager, i, j);

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

  // Schedule the grid swap thread
  if (!kernel::schedule_thread(&grid_swap_thread_tcb)) {
    uart0::puts("Failed to schedule grid swap thread\n");
    return 1;
  }

  uart0::puts("Starting kernel\n");
  kernel::start();

  return 0;
}