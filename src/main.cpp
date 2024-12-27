#include "../include/cells.hpp"
#include "../rpi3-drivers/include/clock.hpp"
#include "../rpi3-drivers/include/framebuffer.hpp"
#include "../rpi3-drivers/include/uart0.hpp"

#include "../transient-os/include/api/concurrency/atomic_guard.hpp"
#include "../transient-os/include/api/sys/sys_calls.hpp"
#include "../transient-os/include/api/thread/thread_handle.hpp"
#include "../transient-os/include/utils/mem_utils.hpp"

/**
 * ### Grid swap thread
 * @brief This thread is responsible for swapping the current and next grids
 * once all the cell threads have finished updating the next grid.
 */
void grid_swap_thread(void *arg) {
  CellGridManager *grid_manager = reinterpret_cast<CellGridManager *>(arg);
  grid_manager->mark_as_ready();

  while (true) {
    if (grid_manager->get_ready_threads() >= CELL_COUNT) {
      AtomicGuard guard;
      grid_manager->swap_grids();
    }

    // Pass control to the next thread
    api::sys::yield();
  }
}

/**
 * ### Cell thread
 * @brief This thread is responsible for updating the state of a single cell
 * in the grid.
 */
void cell_thread(void *arg) {
  Cell *cell = reinterpret_cast<Cell *>(arg);

  // Wait for the grid manager to be ready
  while (!cell->get_grid_manager()->can_start()) {
    api::sys::yield();
  }

  while (true) {
    // Render the cell
    cell->render();

    // Check if the cell is alive
    CellState current_state;

    // Safe get the current state
    {
      AtomicGuard guard;
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
          // AtomicBlock guard;
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
      AtomicGuard guard;
      cell->get_grid_manager()->get_next_grid()->set_cell(
          cell->get_x(), cell->get_y(), next_state);
    }

    // Safe increment the number of ready threads
    {
      AtomicGuard guard;
      cell->get_grid_manager()->increment_ready_threads();
    }

    // Pass control to the next thread
    api::sys::yield();
  }
}

// void test_atomic_swap() {
//   volatile uint32_t shared_value = 42; // Shared value in memory

//   if ((reinterpret_cast<uintptr_t>(&shared_value) & 0x3) != 0) {
//     while (true) {
//       uart0::puts("Shared value is not 4-byte aligned\n");
//     }
//   }

//   // Print MAIR_EL1 register
//   uint64_t mair_el1;
//   asm volatile("mrs %0, mair_el1" : "=r"(mair_el1));

//   uart0::puts("MAIR_EL1: ");
//   uart0::hex(mair_el1);
//   uart0::puts("\n");

//   uint32_t old_value = utils::atomic_swap(&shared_value, 100);

//   uart0::puts("Old value: ");
//   uart0::hex(old_value);
//   uart0::puts("\n");

//   uart0::puts("New value: ");
//   uart0::hex(shared_value);
//   uart0::puts("\n");
// }

int main() {
  uart0::init();
  api::sys::set_output_handler(&uart0::puts);

  if (!framebuffer::init()) {
    uart0::puts("Framebuffer initialization failed\n");
    return 1;
  }

  uart0::puts("Framebuffer initialized\n");

  // test_atomic_swap();

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

  Cell args[GRID_ROWS][GRID_COLS];

  CellGridManager grid_manager;
  grid_manager.init(&grid_a, &grid_b);

  // Initialize cell threads
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      args[i][j].init(&grid_manager, i, j);
    }
  }

  uart0::puts("Declared arrays\n");

  // Spawn cell threads
  ThreadHandle cell_handles[GRID_ROWS][GRID_COLS];

  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      const uint64_t quantum = clock::random_range(1500, 1600);

      if (!api::sys::spawn_kernel_thread(&cell_handles[i][j], &cell_thread,
                                         quantum, &args[i][j])) {
        uart0::puts("Failed to spawn cell thread\n");
        return 1;
      }
    }
  }

  // Spawn the grid swap thread
  ThreadHandle grid_swap_thread_handle;

  if (!api::sys::spawn_kernel_thread(&grid_swap_thread_handle,
                                     &grid_swap_thread, 1000, &grid_manager)) {
    uart0::puts("Failed to spawn grid swap thread\n");
    return 1;
  }

  // Kill the main thread
  return 0;
}