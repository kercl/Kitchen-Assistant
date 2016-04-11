#ifndef WIDGET_H
#define WIDGET_H

#include "../render.h"

class Widget: public RenderableGroup, public EventReceiver {
    static bool mouse_down, draging;
    static Vec2 mouse_down_pos;
    bool enabled;
    Vec2 mouse_position_correction;
  protected:
    std::deque<EventReceiver*> evrcv_children;
  public:    
    virtual EventReceiver* catches_event(const Event &ev);
    virtual void set_color_scheme(const RGBA &base, const RGBA &basedark);
    void set_mouse_position_correction(Vec2 mpc);
    Vec2 get_mouse_position_correction() const;
    
    void registered_callback();
    
    virtual void attach(Widget *);
    virtual void detach(Widget *);
    
    Widget();
    virtual ~Widget();
};

#endif

