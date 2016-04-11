#ifndef CLOCK_H
#define CLOCK_H

#include "widget.h"
#include "../animation.h"

#include <ctime>

class ClockWidget: public Widget, public Animator {
protected:
  FramebufferPass fbp;
  Rect r;
  bool init_flag;
  Vec2 pos;
  float rad;
  tm *ltm;
  
  RenderableGroup *sec_grp, *min_grp, *hour_grp;
  
  class SecondHandAnimator: public Animator {
    ClockWidget *parent;
    float cum_elapsed;
  public:
    float reference_time;
    SecondHandAnimator(ClockWidget *parent);
    void step();
  };
  
  SecondHandAnimator sha;
  ClockWidget(Vec2 position, float radius, bool empty);
public:
  ClockWidget(Vec2 position, float radius);
  
  void on_mouse_click(const MouseEvent &ev);
  int get_year();
  int get_month();
  int get_day();
  int get_day_of_week();
  
  void step();
  
  EventReceiver* catches_event(const Event &ev);
  void issue_event(const Event &ev);
};

class MinimalisticClockWidget: public ClockWidget {
public:
  MinimalisticClockWidget(Vec2 position, float radius);
};

#endif

