#ifndef SLIDER_H
#define SLIDER_H

#include "widget.h"
#include "../box.h"
#include "../geom.h"

class Slider: public Widget {
public:
    enum orientation {
      vertical_orientation,
      horizontal_orientation
    };
protected:
    Box *bg, *rangebox;
    float value, extend;
    orientation _orient;
    bool invert;
    Circle *circ;
    int w,h;
    
    void update_position();
public:  
    Slider(int w, int h, orientation o, float initval=0.0f);
    
    virtual void on_mouse_down(const MouseEvent &ev);
    virtual void on_mouse_drag_motion(const MouseEvent &ev);
    //void on_drag_end(const MouseEvent &ev);
    EventReceiver* catches_event(const Event &ev);
    
    void set_invert(bool i);
    void set_color_scheme(const RGBA &base, const RGBA &basedark);
    float get_value();
    void set_value(float f, bool call_callback = true);
    
    virtual void callback();
};

#endif

