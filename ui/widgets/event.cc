
#include "../render.h"
#include "event.h"
#include <iostream>

using namespace std;

EventType Event::get_event_type() const {}

EventType MouseEvent::get_event_type() const {
  return EventTypeMouse;
}

MouseEvent::MouseEvent(int _button, int _x, int _y) 
: p(Vec2(_x, _y)), button(_button)
{}

int MouseEvent::get_x() const { return p.x; }
int MouseEvent::get_y() const { return p.y; }
void MouseEvent::set_x(int _x) { p.x = _x; }
void MouseEvent::set_y(int _y) { p.y = _y;}
int MouseEvent::get_button() const { return button; }
Vec2 MouseEvent::get_cursor() const { return p; }
void MouseEvent::set_cursor(Vec2 _p) { p = _p; }
Vec2 MouseEvent::get_relative_cursor() const { return relp; }
void MouseEvent::set_relative_cursor(Vec2 _p) { relp = _p; }

EventType MouseDragEvent::get_event_type() const {
  return EventTypeMouseDrag;
}
MouseDragEvent::MouseDragEvent(int _x, int _y): MouseEvent(0,_x,_y) {}

EventType MouseClickEvent::get_event_type() const {
  return EventTypeMouseClick;
}
MouseClickEvent::MouseClickEvent(int button, int x, int y): MouseEvent(button,x,y) {}

EventType MouseDownEvent::get_event_type() const {
  return EventTypeMouseDown;
}
MouseDownEvent::MouseDownEvent(int button, int x, int y): MouseEvent(button,x,y) {}

EventType MouseUpEvent::get_event_type() const {
  return EventTypeMouseUp;
}
MouseUpEvent::MouseUpEvent(int button, int x, int y): MouseEvent(button,x,y) {}

EventType MouseMoveEvent::get_event_type() const {
  return EventTypeMouseMove;
}
MouseMoveEvent::MouseMoveEvent(int x, int y): MouseEvent(-1,x,y) {}

EventReceiver::EventReceiver(): can_rcv(true) {}

void EventReceiver::disable() { can_rcv = false; }
void EventReceiver::enable() { can_rcv = true; }

bool EventReceiver::can_receive() {
  return can_rcv;
}

void EventReceiver::on_mouse_event(const MouseEvent &ev) {}
void EventReceiver::on_mouse_click(const MouseEvent &ev) {}
void EventReceiver::on_mouse_drag_motion(const MouseEvent &ev) {}
void EventReceiver::on_mouse_down(const MouseEvent &ev) {}
void EventReceiver::on_mouse_up(const MouseEvent &ev) {}
void EventReceiver::on_drag_start(const MouseEvent &ev) {}
void EventReceiver::on_drag_end(const MouseEvent &ev) {}
void EventReceiver::on_window_mouse_up(const MouseEvent &ev) {}

EventHandler::EventHandler()
:mousedown(false), draging(false), mousedownpos(0,0), mousedownobj(NULL) {}

void EventHandler::evoke(const Event &ev) {
  //cout << "evoking: " << ev.get_event_type() << endl;
  if(ev.get_event_type() == EventTypeMouseDown) {
    mousedownobj = NULL;
  }else if(ev.get_event_type() == EventTypeMouseUp) {
    if(draging) {
      //cout << "on_drag_end " << mousedownobj << endl;
      //mousedownobj->on_drag_end(dynamic_cast<const MouseEvent&>(ev));
    }
    //draging = false;
    mousedown = false;
  }
  
  if(ev.get_event_type() == EventTypeMouseMove)
    if(draging && mousedownobj) {
      mousedownobj->on_mouse_drag_motion(dynamic_cast<const MouseEvent&>(ev));
    }
  
  for(auto x: recvr) {
    if(!x->can_receive())
      continue;
  
    EventReceiver *r = x->catches_event(ev);
    
    if(ev.get_event_type() == EventTypeMouseUp && r)
      r->on_window_mouse_up(dynamic_cast<const MouseEvent&>(ev));
    
    if(r) {  
      if(ev.get_event_type() <= EventTypeMouse)
        evoke_mouse_event(dynamic_cast<const MouseEvent&>(ev), r);
      
      break;
    }
  }
}

void EventHandler::register_receiver(EventReceiver *rcv) {
  //cout << "register receiver "<< rcv << endl;
  recvr.push_front(rcv);
}

void EventHandler::evoke_mouse_event(const MouseEvent &ev, EventReceiver *target) {
  if(ev.get_event_type() == EventTypeMouseDown) {
    target->on_mouse_down(ev);
  
    mousebtn = ev.get_button();
    mousedownpos = ev.get_cursor();
    mousedownobj = target;
    
    mousedown = true;
  }else if(ev.get_event_type() == EventTypeMouseUp) {
    if(mousedownobj == target && !draging) 
      target->on_mouse_click(ev);
    if(draging) {
      mousedownobj->on_drag_end(ev);
    }
    draging = false;
    mousedown = false;
    
    target->on_mouse_up(ev);
    
    mousedownobj = NULL;
  }else if(ev.get_event_type() == EventTypeMouseMove) {
    if(mousedown) {
      if( (mousedownpos - ev.get_cursor()).length() > 10 || draging ) {
        if(draging == false) {
          mousedownobj->on_drag_start(ev);
        }
        draging = true;
      }
    }
  }
}

