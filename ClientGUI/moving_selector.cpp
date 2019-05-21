#include "moving_selector.h"
#include "board_widget.h"
#include <cassert>

MovingSelectManager::MovingSelectManager() : edge_to_move_(nullptr) {}

void MovingSelectManager::set_edge(EdgeButton* edge)
{
  assert(edge);
  clear_edge();
  edge_to_move_ = edge;
  edge->select();
}

void MovingSelectManager::clear_edge()
{
  if (edge_to_move_)
  {
    edge_to_move_->cancel_select();
    edge_to_move_ = nullptr;
  }
}

EdgeButton* MovingSelectManager::get_edge()
{
  return edge_to_move_;
}
