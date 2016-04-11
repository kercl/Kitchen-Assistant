#ifndef TIMESELECTOR_H
#define TIMESELECTOR_H

#include "widget.h"
#include "../geom.h"
#include "../text.h"

#define MINS_DETAIL_ANGLE 0.8f

class TimeSelector: public Widget, public Animator {
  Vec2 pos;
  
  Circle *mins[12], *minsdetail[4],
         *secs[12], *secsdetail[4], **target;
  int min_select, sec_select, min_sub_select;
  Text *timestring;
  float min_detail_alpha;
  float min_sel_space[12];
public:
  TimeSelector(const Vec2 &shape);

  EventReceiver* catches_event(const Event &ev);
  
  void on_mouse_drag_motion(const MouseEvent &ev);
  void on_mouse_down(const MouseEvent &ev);
  void on_mouse_up(const MouseEvent &ev);
  
  void step();
  
  void set_seconds(int s);
  int get_seconds();
};

#endif

