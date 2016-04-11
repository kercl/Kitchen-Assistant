#include "slider.h"
#include <algorithm>
#include <iostream>

using namespace std;

Slider::Slider(int _w, int _h, orientation o, float initval)
: _orient(o), 
  value(initval),
  invert(false),
  w(_w),
  h(_h)
{
  Rect shape(0, 0, w, h);
  bg = new Box(shape);
  bg->set_background_color(RGBA(1,1,1,1));
  RenderableGroup::attach(bg);
  
  if(invert)
    value = 1.0f - value;
  
  
  if(_orient == vertical_orientation) {
    rangebox = new Box(Rect(w/2-2, 20, w/2+2, h-20));
    extend = h - 40;
  }else {
    rangebox = new Box(Rect(20, h/2-2, w-20, h/2+2));
    extend = w - 40;
  }
  
  if(invert)
    value = 1.0f - value;
  
  rangebox->set_background_color(RGBA(1,1,1,0.9));
  RenderableGroup::attach(rangebox);
  
  circ = new Circle(Vec2(0,0), 7, 16);
  circ->set_blendcol(RGBA(1,1,1,1));
  RenderableGroup::attach(circ);
  
  update_position();
}
/*
void Slider::on_drag_end(const MouseEvent &ev) {
  cout << "sdmaskld" << endl;
}*/

void Slider::set_invert(bool i) {
  invert = i;
  update_position();
}

void Slider::update_position() {
  if(invert)
    value = 1.0f - value;
  
  if(_orient == vertical_orientation)
    circ->set_center(Vec2(w/2,20 + extend*value));
  else
    circ->set_center(Vec2(20 + extend*value,h/2));
  
  if(invert)
    value = 1.0f - value;
}

float Slider::get_value() {
  return value;
}

void Slider::set_value(float v, bool call_callback) {
  value = std::min(1.0f, std::max(0.0f, v));
  
  update_position();
  if(call_callback)
    callback();
  Renderable::context->refresh();
}

void Slider::set_color_scheme(const RGBA &base, const RGBA &basedark) {
  bg->set_background_color(basedark);
  bg->set_background_alpha(0.6);
}

void Slider::on_mouse_down(const MouseEvent &ev) {
  Vec2 cursor = ev.get_cursor() - get_origin() - get_mouse_position_correction();
  if(_orient == vertical_orientation) {
    cursor.y -= 20;
    value = min(1.0f, max(0.0f, (float)cursor.y / extend));
  }
  if(invert)
    value = 1.0f - value;
  
  update_position();
  
  callback();
  Renderable::context->refresh();
}

void Slider::on_mouse_drag_motion(const MouseEvent &ev) {
  on_mouse_down(ev);
}

void Slider::callback() {}

EventReceiver* Slider::catches_event(const Event &ev) {
  if(ev.get_event_type() <= EventTypeMouse)
    if(bg->get_shape().contains(dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin()))
      return this;
  return NULL;
}
