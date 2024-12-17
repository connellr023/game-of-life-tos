#include "../include/grid.hpp"
#include "../rpi3-drivers/include/framebuffer.hpp"

void CellThreadArg::render() {
  const uint32_t color = this->get_current_state() == CellState::Alive
                             ? CELL_ALIVE_COLOR
                             : CELL_DEAD_COLOR;

  const int screen_x = this->x * CELL_PIXEL_SIZE;
  const int screen_y = this->y * CELL_PIXEL_SIZE;
  const int screen_x_end = screen_x + CELL_PIXEL_SIZE - 1;
  const int screen_y_end = screen_y + CELL_PIXEL_SIZE - 1;

  framebuffer::draw_rect(screen_x, screen_y, screen_x_end, screen_y_end, color);
}

CellState CellThreadArg::get_current_state() {
  return this->grid_manager->get_current_grid()->get_cell(x, y);
}