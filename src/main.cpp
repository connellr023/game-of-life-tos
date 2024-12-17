#include "../include/grid.hpp"
#include "../rpi3-drivers/include/clock.hpp"
#include "../rpi3-drivers/include/framebuffer.hpp"
#include "../rpi3-drivers/include/uart0.hpp"
#include "../transient-os/include/kernel/kernel.hpp"
#include "../transient-os/include/utils/concurrency/atomic.hpp"

using namespace kernel::threads;

/**
 * Grid swap thread
 * This thread is responsible for swapping the current and next grids
 * once all the cell threads have finished updating the next grid.
 */
void grid_swap_thread(void *arg) {
  GridManager *grid_manager = reinterpret_cast<GridManager *>(arg);

  while (true) {
    if (grid_manager->get_ready_threads() >= 2) {
      AtomicBlock guard;
      grid_manager->swap_grids();
    }

    kernel::yield();
  }
}

/**
 * Cell thread
 * This thread is responsible for updating the state of a single cell
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

        const int x = cell->x + i;
        const int y = cell->y + j;

        if (x < 0 || x >= GRID_ROWS || y < 0 || y >= GRID_COLS) {
          continue;
        }

        CellState state;

        {
          AtomicBlock guard;
          state = cell->grid_manager->get_current_grid()->get_cell(x, y);
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
      cell->grid_manager->get_next_grid()->set_cell(cell->x, cell->y,
                                                    next_state);
    }

    // Safe increment the number of ready threads
    {
      AtomicBlock guard;
      cell->grid_manager->increment_ready_threads();
    }

    // Pass control to the next thread
    kernel::yield();
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
  CellGrid grid_a = CellGrid();
  CellGrid grid_b = CellGrid();

  // Randomly initialize the grid
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      const uint64_t seed = clock::current_micros();
      grid_a.set_cell(i, j, seed % 2 == 0 ? CellState::Alive : CellState::Dead);
    }
  }

  CellThreadArg args[GRID_ROWS][GRID_COLS] = {{CellThreadArg()}};
  ThreadControlBlock threads[GRID_ROWS][GRID_COLS] = {{ThreadControlBlock()}};

  GridManager grid_manager = GridManager(&grid_a, &grid_b);
  ThreadControlBlock grid_swap_thread_tcb =
      ThreadControlBlock(&grid_swap_thread, 2000, &grid_manager);

  uart0::puts("Declared arrays\n");

  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      const uint64_t burst_time = clock::random_range(1800, 2500);
      threads[i][j] = ThreadControlBlock(&cell_thread, burst_time, &args[i][j]);

      CellThreadArg *cell = &args[i][j];
      cell->grid_manager = &grid_manager;
      cell->x = i;
      cell->y = j;

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