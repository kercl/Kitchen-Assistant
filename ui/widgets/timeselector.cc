#include "timeselector.h"
#include "../box.h"
#include "../geom.h"
#include "../text.h"

#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;


TimeSelector::TimeSelector(const Vec2 &_pos)
: pos(_pos),
  Animator(10),
  min_select(0),
  sec_select(0),
  min_sub_select(0),
  target(NULL),
  min_detail_alpha(0.f)
{
  Circle *circ = new Circle(_pos,190,40);
  circ->set_blendcol(RGBA("#95a5a6", 1));
  RenderableGroup::attach(circ);
  circ = new Circle(_pos,115,40);
  circ->set_blendcol(RGBA("#ecf0f1", 1));
  RenderableGroup::attach(circ);
  
  for(int i = 0; i < 12; ++i) {
    Vec2 dir = Vec2(sin(2.f * M_PI * i / 12.f), -cos(2.f * M_PI * i / 12.f));
    mins[i] = new Circle(pos + dir*180, 25, 30);
    mins[i]->set_blendcol(RGBA("#95a5a6", 1));
    RenderableGroup::attach(mins[i]);
    
    secs[i] = new Circle(pos + dir*105, 20, 30);
    secs[i]->set_blendcol(RGBA("#ecf0f1", 1));
    RenderableGroup::attach(secs[i]);
    min_sel_space[i] = 0.f;
  }
  
  for(int i = 0; i < 4; ++i) {
    minsdetail[i] = new Circle(pos, 7, 30);
    minsdetail[i]->set_blendcol(RGBA("#ecf0f1", 1));
    minsdetail[i]->set_border_color(RGBA("#95a5a6", 1));
    minsdetail[i]->set_border_width(7);
    minsdetail[i]->hide();
    RenderableGroup::attach(minsdetail[i]);
  }
  
  mins[0]->set_radius(40); // 25
  secs[0]->set_radius(32); // 20
  
  timestring = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 37), pos-Vec2(75,20));
  timestring->set_blendcol(RGBA(0,0,0,0.65));
  timestring->get_stream() << "00'00\"";
  RenderableGroup::attach(timestring);
  
  Renderable::context->register_animator(this);
  pause();
}

EventReceiver* TimeSelector::catches_event(const Event &ev) {
  if(ev.get_event_type() <= EventTypeMouse)
    if((dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin() - pos).length() < 220)
      return this;
  return NULL;
}

void TimeSelector::set_seconds(int s) {
  sec_select = (s % 60) / 5;
  min_select = (s / 60) / 5;
  min_sub_select = (s / 60) % 5;
  timestring->get_stream().str(string());
  timestring->get_stream() << setfill('0') 
                           << setw(2) 
                           << (5 * min_select + min_sub_select) << "'" 
                           << setfill('0') 
                           << setw(2) 
                           << 5 * sec_select << "\"";
  target = NULL;
  
  MouseEvent ev(1, 0, 130);
  on_mouse_down(ev);
}

int TimeSelector::get_seconds() {
  return (5 * min_select + min_sub_select) * 60 + 5 * sec_select;
}

void TimeSelector::step() {
  if(target == NULL) {
    pause();
    return;
  }

  int stop = 0;
  
  timestring->get_stream().str(string());
  timestring->get_stream() << setfill('0') 
                           << setw(2) 
                           << (5 * min_select + min_sub_select) << "'" 
                           << setfill('0') 
                           << setw(2) 
                           << 5 * sec_select << "\"";
  
  float bigger = 40, smaller = 25;
  int selection = min_select, stop_counter_end = 24;
  if(target == secs) {
    bigger = 32;
    smaller = 20;
    stop_counter_end = 12;
    selection = sec_select;
  }
  
  float min_details_full_angle = 0.f;
  for(int i = 0; i < 12; ++i) {
    float endspace = (i==min_select ? MINS_DETAIL_ANGLE : 0);
    float dr = (endspace - min_sel_space[i]) / 8.f;
    
    min_sel_space[i] += dr;
    if(abs(dr) < 0.001f) {
      min_sel_space[i] = endspace;
      stop++;
    }
    
    min_details_full_angle += min_sel_space[i];
  }
  
  float selangle, alpha = 0.f;
  for(int i = 0; i < 12; ++i) {
    float endrad = (i==selection ? bigger : smaller);
    float dr = (endrad - target[i]->get_radius()) / 3.f;
    
    if(i == selection)
      selangle = alpha;
    
    if(target == mins) {
      Vec2 dir = Vec2(sin(alpha), -cos(alpha));
      target[i]->set_center(pos + dir*180);
      
      alpha += (2.f * M_PI - min_details_full_angle) / 12.f + min_sel_space[i];
    }
  
    endrad = (i==selection ? bigger : smaller);
    dr = (endrad - target[i]->get_radius()) / 5.f;
    target[i]->set_radius(dr + target[i]->get_radius());
    if(abs(dr) < 0.1f) {
      target[i]->set_radius(endrad);
      stop++;
    }
  }
  if(target == mins)
    for(int i = 0; i < 4; ++i) {
      minsdetail[i]->set_center(
        pos + Vec2(sin(selangle + (i + 1) * min_sel_space[selection] / 3.5f+0.1f), 
                  -cos(selangle + (i + 1) * min_sel_space[selection] / 3.5f+0.1f)) * 185);
      minsdetail[i]->show();
    }

  if(stop == stop_counter_end) {
    /*if(target == mins) {
      MouseEvent ev(1, pos.x, pos.y);
      on_mouse_down(ev);
    }else*/
      pause();
  }
}

void TimeSelector::on_mouse_drag_motion(const MouseEvent &ev) {
  if(target == NULL)
    return;

  Vec2 relpos = dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin() - pos;
  float mindist = 40.f;
  
  for(int i = 0; i < 12; ++i) {
    if((relpos - target[i]->get_center() + pos).length() < mindist) {
      mindist = (relpos - target[i]->get_center() + pos).length();
      if(target == mins) {
        min_select = i;
        min_sub_select = 0;
      }else
        sec_select = i;
    }
  }
  
  play();
}

void TimeSelector::on_mouse_down(const MouseEvent &ev) {
  Vec2 relpos = dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin() - pos;

  target = mins;
  if(relpos.length() < 130)
    target = secs;
  
  bool detail_selected = false;
  if(target == mins)
    for(int i = 0; i < 4; ++i) {
      if((relpos - minsdetail[i]->get_center() + pos).length() < 22) {
        min_sub_select = i + 1;
        detail_selected = true;
      }
    }
  
  if(!detail_selected)
    on_mouse_drag_motion(ev);
}

void TimeSelector::on_mouse_up(const MouseEvent &ev) {
  if(target == NULL)
    return;
  play();
}

