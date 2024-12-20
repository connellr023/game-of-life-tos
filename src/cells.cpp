#include "../include/cells.hpp"
#include "../rpi3-drivers/include/framebuffer.hpp"

void Cell::render() const {
  const uint32_t color = this->get_current_state() == CellState::Alive
                             ? CELL_ALIVE_COLOR
                         : (this->x + this->y) % 2 == 0 ? CELL_DEAD_COLOR_1
                                                        : CELL_DEAD_COLOR_2;

  const int screen_x = this->x * CELL_PIXEL_SIZE;
  const int screen_y = this->y * CELL_PIXEL_SIZE;
  const int screen_x_end = screen_x + CELL_PIXEL_SIZE - 1;
  const int screen_y_end = screen_y + CELL_PIXEL_SIZE - 1;

  framebuffer::draw_rect(screen_x, screen_y, screen_x_end, screen_y_end, color);
}

CellGrid *CellGridManager::get_current_grid() {
  if (this->current_grid) {
    return this->grid_a;
  } else {
    return this->grid_b;
  }
}

CellGrid *CellGridManager::get_next_grid() {
  if (this->current_grid) {
    return this->grid_b;
  } else {
    return this->grid_a;
  }
}

void CellGridManager::swap_grids() {
  this->current_grid = !this->current_grid;
  this->ready_threads = 0;
}