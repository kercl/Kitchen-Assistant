#ifndef EVENT_H
#define EVENT_H

#include "../utils.h"
#include <deque>

class Renderable;

enum EventType {
  EventTypeMouseDrag,
  EventTypeMouseClick,
  EventTypeMouseDown,
  EventTypeMouseUp,
  EventTypeMouseMove,
  EventTypeMouse,  // end of mouse event block (order important)
  EventEmpty,
};

class Event {
public:
  virtual EventType get_event_type() const;
};

class MouseEvent: public Event {
protected:
  int button;
  Vec2 p, relp;
public:
  virtual EventType get_event_type() const;
  MouseEvent(int _button, int _x, int _y);
  
  int get_x() const;
  int get_y() const;
  void set_x(int _x);
  void set_y(int _y);
  int get_button() const;
  Vec2 get_cursor() const;
  void set_cursor(Vec2 _p);
  
  Vec2 get_relative_cursor() const;
  void set_relative_cursor(Vec2 _p);
};

class MouseDragEvent: public MouseEvent {
public:
  EventType get_event_type() const;
  MouseDragEvent(int _x, int _y);
};

class MouseClickEvent: public MouseEvent {
public:
  EventType get_event_type() const;
  MouseClickEvent(int button, int x, int y);
};

class MouseDownEvent: public MouseEvent {
public:
  EventType get_event_type() const;
  MouseDownEvent(int button, int x, int y);
};

class MouseUpEvent: public MouseEvent {
public:
  EventType get_event_type() const;
  MouseUpEvent(int button, int x, int y);
};

class MouseMoveEvent: public MouseEvent {
public:
  EventType get_event_type() const;
  MouseMoveEvent(int x, int y);
};

class EventReceiver {
  bool can_rcv;
public:
  EventReceiver();
  
  void disable();
  void enable();
  
  bool can_receive();

  virtual EventReceiver* catches_event(const Event &ev) = 0;

  virtual void on_mouse_event(const MouseEvent &ev);
  virtual void on_mouse_click(const MouseEvent &ev);
  virtual void on_mouse_drag_motion(const MouseEvent &ev);
  virtual void on_mouse_down(const MouseEvent &ev);
  virtual void on_mouse_up(const MouseEvent &ev);
  virtual void on_drag_start(const MouseEvent &ev);
  virtual void on_drag_end(const MouseEvent &ev);
  virtual void on_window_mouse_up(const MouseEvent &ev);
};

class EventHandler {
  std::deque<EventReceiver*> recvr;
  
  bool mousedown, draging;
  Vec2 mousedownpos;
  int mousebtn;
  EventReceiver* mousedownobj;
public:
  EventHandler();

  void evoke_mouse_event(const MouseEvent &ev, EventReceiver *target);
  void evoke(const Event &ev);
  
  void register_receiver(EventReceiver *rcv);
};

#endif

