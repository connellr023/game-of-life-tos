#ifndef CELL_HPP
#define CELL_HPP

#include "grid.hpp"

#define CELL_PIXEL_SIZE 16
#define CELL_ALIVE_COLOR 0xFFFFFF
#define CELL_DEAD_COLOR 0x000000

struct CellThreadArg {
public:
  GridManager *grid_manager;

  int x;
  int y;

  CellState get_current_state() {
    return grid_manager->get_current_grid()->get_cell(x, y);
  }

  void render();
};

#endif // CELL_HPP