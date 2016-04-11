#include "button.h"
#include <algorithm>
#include <iostream>

using namespace std;

Button::Button(int w, int h, Img *img): Animator(10) {
  highlight = 0.6;
  
  Rect shape(0, 0, w, h);
  bg = new Box(shape);
  bg->set_background_color(RGBA(1,1,1,highlight));
  
  RenderableGroup::attach(bg);
  if(img) {
    img->set_shape(Rect(
      (w - img->get_width()) * 0.5,
      (h - img->get_height()) * 0.5,
      (w + img->get_width()) * 0.5,
      (h + img->get_height()) * 0.5));
    RenderableGroup::attach(img);
  }
  image = img;
  caption = NULL;
  height = h;
}

Button::Button(int w, int h, std::string text): Button(w, h, NULL) {
  caption = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 13), Vec2(12, (h - 13)*0.5));
  caption->set_blendcol(RGBA(0,0,0,0.7));
  caption->get_stream() << text;
  RenderableGroup::attach(caption);
}

void Button::set_color_scheme(const RGBA &base, const RGBA &basedark) {
  bg->set_background_color(RGBA(basedark.red, basedark.green, basedark.blue, highlight));
}

void Button::set_dimensions(int w, int h) {
  if(image) {
    image->set_shape(Rect(
      (w - image->get_width()) * 0.5,
      (h - image->get_height()) * 0.5,
      (w + image->get_width()) * 0.5,
      (h + image->get_height()) * 0.5));
  }
  bg->set_shape(Rect(0, 0, w, h));
}

void Button::on_mouse_click(const MouseEvent &ev) {
/*  highlight = 1.0;
  Renderable::context->register_animator(this);*/
  callback();
}

void Button::on_mouse_down(const MouseEvent &ev) {
  highlight = 1.0;
  Renderable::context->register_animator(this);
  //callback();
}

void Button::set_text(std::string t) {
  if(!caption)
    return;
  
  caption->get_stream().str(t);
}

void Button::step() {
  highlight -= 0.02;
  if(highlight < 0.6) {
    highlight = 0.6;
    unregister();
  }
  bg->set_background_alpha(highlight);
}

EventReceiver* Button::catches_event(const Event &ev) {
  //cout << ev.get_event_type() << ": " << bg->get_shape().left << " - " << bg->get_shape().right << endl;
  /*if(bg->get_shape().contains(ev.get_cursor() - get_origin()))
    return this;*/
  if(ev.get_event_type() <= EventTypeMouse)
    if(bg->get_shape().contains(dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin()))
      return this;
  return NULL;
}
