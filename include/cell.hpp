#ifndef CELL_HPP
#define CELL_HPP

#include "grid.hpp"

#define CELL_PIXEL_SIZE 30
#define CELL_ALIVE_COLOR 0x00FF00
#define CELL_DEAD_COLOR 0x0000FF

struct CellThreadArg {
public:
  CellGrid *current_grid;
  CellGrid *next_grid;
  int x;
  int y;

  CellState get_current_state() {
    return current_grid->get_cell(this->x, this->y);
  }

  void render();
};

#endif // CELL_HPP