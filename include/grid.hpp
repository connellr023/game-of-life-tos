#ifndef GRID_HPP
#define GRID_HPP

#include <stdint.h>

#define GRID_ROWS 15
#define GRID_COLS 16
#define CELL_COUNT (GRID_ROWS * GRID_COLS)

#define CELL_PIXEL_SIZE 16
#define CELL_ALIVE_COLOR 0xFFFFFF
#define CELL_DEAD_COLOR 0x000000

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
  void init(CellGrid *grid_a, CellGrid *grid_b) {
    this->current_grid = true;
    this->ready_threads = 0;
    this->grid_a = grid_a;
    this->grid_b = grid_b;
  }

  CellGrid *get_current_grid();
  CellGrid *get_next_grid();
  void swap_grids();

  uint32_t get_ready_threads() { return this->ready_threads; }
  void increment_ready_threads() { this->ready_threads++; }
};

class CellThreadArg {
private:
  GridManager *grid_manager;

  int x;
  int y;

public:
  void init(GridManager *grid_manager, int x, int y) {
    this->grid_manager = grid_manager;
    this->x = x;
    this->y = y;
  }

  int get_x() const { return this->x; }
  int get_y() const { return this->y; }

  CellState get_current_state() const {
    return this->grid_manager->get_current_grid()->get_cell(x, y);
  }

  GridManager *get_grid_manager() { return this->grid_manager; }

  void render() const;
};

#endif // GRID_HPP