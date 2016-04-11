#include "widget.h"

#include <iostream>

using namespace std;

bool Widget::mouse_down = false, Widget::draging = false;
Vec2 Widget::mouse_down_pos = Vec2();

//Widget::Widget(): mouse_down(false), enabled(true) {}
Widget::Widget() {
  //cout << "new widget" << endl;
}

Widget::~Widget() {}

void Widget::set_color_scheme(const RGBA &base, const RGBA &basedark) {}

void Widget::set_mouse_position_correction(Vec2 mpc) {
  mouse_position_correction = mpc;
}

Vec2 Widget::get_mouse_position_correction() const {
  return mouse_position_correction;
}

void Widget::registered_callback() {
  Renderable::context->register_receiver(this);
}

void Widget::attach(Widget *w) {
  for(int i = 0; i < evrcv_children.size(); ++i)
    if(evrcv_children[i] == NULL) {
      evrcv_children[i] = w;
      return;
    }
  evrcv_children.push_front(w);
  RenderableGroup::attach(w);
}

void Widget::detach(Widget *w) {
  for(int i = 0; i < evrcv_children.size(); ++i)
    if(evrcv_children[i] == w) {
      evrcv_children[i] = NULL;
      break;
    }
  RenderableGroup::detach(w);
}

EventReceiver* Widget::catches_event(const Event &ev) {
  for(auto x: evrcv_children) {
    if(x == NULL)
      continue;
  
    if(x->can_receive() == false)
      continue;
    EventReceiver *cx = x->catches_event(ev);
    if(cx)
      return cx;
  }
  return NULL; 
}

