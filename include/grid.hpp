#ifndef GRID_HPP
#define GRID_HPP

#define GRID_ROWS 5
#define GRID_COLS 5

enum class CellState {
  Dead,
  Alive,
};

class CellGrid {
private:
  CellState grid[GRID_ROWS][GRID_COLS] = {{CellState::Dead}};

public:
  CellState get_cell(int x, int y) { return grid[x][y]; }
  void set_cell(int x, int y, CellState state) { grid[x][y] = state; }
};

#endif // GRID_HPP