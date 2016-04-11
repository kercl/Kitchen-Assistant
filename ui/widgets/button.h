#ifndef BUTTON_H
#define BUTTON_H

#include <cstring>

#include "widget.h"
#include "../animation.h"
#include "../img.h"
#include "../box.h"

class Button: public Widget, public Animator {
  protected:
    Box *bg;
    float highlight, height;
    Img *image;
    Text *caption;
  public:
    Button(int w, int h, Img *img);
    Button(int w, int h, std::string text);
    
    void on_mouse_click(const MouseEvent &ev);
    void on_mouse_down(const MouseEvent &ev);
    virtual EventReceiver* catches_event(const Event &ev);
    
    void set_color_scheme(const RGBA &base, const RGBA &basedark);
    void set_dimensions(int w, int h);
    
    virtual void step();
    virtual void set_opacity_factor(float f) {
      bg->set_opacity_factor(f);
      if(image)
        image->set_opacity_factor(f);
      if(caption)
        caption->set_opacity_factor(f);
    }
    
    virtual void callback() = 0;
    
    void set_text(std::string t);
};

#endif

