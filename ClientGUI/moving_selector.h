#pragma once

class EdgeButton;

class MovingSelectManager
{
private:
  EdgeButton* edge_to_move_;
public:
  MovingSelectManager();
  void set_edge(EdgeButton* edge);
  void clear_edge();
  EdgeButton* get_edge();
};
