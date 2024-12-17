#ifndef GRID_HPP
#define GRID_HPP

#define GRID_ROWS 15
#define GRID_COLS 16
#define CELL_COUNT (GRID_ROWS * GRID_COLS)

#include <stdint.h>

enum class CellState {
  Dead,
  Alive,
};

class CellGrid {
private:
  CellState grid[GRID_ROWS][GRID_COLS] = {{CellState::Dead}};

public:
  CellState get_cell(int x, int y) { return this->grid[x][y]; }
  void set_cell(int x, int y, CellState state) { this->grid[x][y] = state; }
};

class GridManager {
private:
  bool current_grid;
  uint32_t ready_threads;

  CellGrid *grid_a;
  CellGrid *grid_b;

public:
  GridManager(CellGrid *grid_a, CellGrid *grid_b)
      : current_grid(true), ready_threads(0), grid_a(grid_a), grid_b(grid_b) {}

  CellGrid *get_current_grid() {
    if (this->current_grid) {
      return this->grid_a;
    } else {
      return this->grid_b;
    }
  }

  CellGrid *get_next_grid() {
    if (this->current_grid) {
      return this->grid_b;
    } else {
      return this->grid_a;
    }
  }

  uint32_t get_ready_threads() { return this->ready_threads; }
  void increment_ready_threads() { this->ready_threads++; }

  void swap_grids() {
    this->current_grid = !this->current_grid;
    this->ready_threads = 0;
  }
};

#endif // GRID_HPP