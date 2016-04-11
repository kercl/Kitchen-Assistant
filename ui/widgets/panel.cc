#include "panel.h"
#include "button.h"
#include "../img.h"
#include <iostream>
#include <cmath>

using namespace std;

class TestButton: public Button {
public:
  TestButton(int w, int h, string s): Button(w,h,s) {}
  void callback() {};
};

class ReturnButton: public Button {
  AppPanel *main;
public:
  ReturnButton(AppPanel *_main): Button(72, 72, NULL) {
    main = _main;
    
    //lbar = new Box(Rect(0,0,72,72));
    //lbar->set_background_color(main->darkbase);
    //lbar->set_border_color(RGBA(0,0,0,0.8));
    //lbar->set_border_width(1.0);
    //RenderableGroup::attach(lbar);
    
    Img *leftarrow = new Img("data/widgets/back.png", Rect(20,20,72-20,72-20));
    leftarrow->set_blendcol(RGBA(1,1,1,0.8));
    RenderableGroup::attach(leftarrow);
    //cout << "ReturnButton " << this << endl;
  }
  
  void callback() {
    main->disable();
    main->hide_panel();
  }
};

Box* AppPanel::get_background() {
  return bg;
}

AppPanel::AppPanel(RGBA b, RGBA db)
: Animator(10),
  blend_state(-2.0),
  toggle_state(false),
  home(NULL),
  base(b),
  darkbase(db)
{
  bg = new Box(Rect(0,0,1024,600));
  bg->set_background_color(RGBA("#ecf0f1",1));
  //bg->set_blendcol(RGBA(1,1,1,0.5));
  //bg->drop_shadow(Vec2(-1,0),5,RGBA(0,0,0,0.4));
  /*bg->append_texture(new LinearGradientTexture({
    {0, RGBA(0,0,0,0.1)},
    {15,RGBA(0,0,0,0.15)},
    {20,RGBA(0,0,0,0.4)},
    {22,RGBA(0,0,0,0.6)},
    {24,RGBA(0,0,0,0.8)}}),
    TexCoords(Vec2(0,0),Vec2(1,0),Vec2(1,1),Vec2(0,1)));
  /*bg->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {100,RGBA(1,1,1,0.0)}}),
    TexCoords(Vec2(0.2,0.2),Vec2(2,0.2),Vec2(2,1.5),Vec2(0.2,1.5)));
  bg->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {100,RGBA(1,1,1,0.0)}}),
    TexCoords(Vec2(0,0),Vec2(0.5,0),Vec2(0.5,0.5),Vec2(0,0.5)));
  bg->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {100,RGBA(1,1,1,0.0)}}),
    TexCoords(Vec2(-1.5,-0.1),Vec2(1.1,-0.1),Vec2(1.1,1.3),Vec2(-1.5,1.3)));*/
  RenderableGroup::attach(bg);
  
  Box *lbar;
  lbar = new Box(Rect(0,0,72,600));
  lbar->set_background_color(base);
  lbar->set_blendcol(RGBA(1,1,1,0.6));
  RenderableGroup::attach(lbar);
  
  ReturnButton *retbtn = new ReturnButton(this);
  attach(retbtn);
  
  translate(Vec2(1024,0));
  
  /*
  Box *lbar = new Box(Rect(0,0,72,600));
  lbar->set_background_color(RGBA(1,1,1,0.5));
  lbar->set_border_color(RGBA(0,0,0,0.8));
  lbar->set_border_width(1.0);
  attach(lbar);
  
  Img *leftarrow = new Img("data/navigation/directional.png", Rect(4,300-32,4+64,300+32));
  leftarrow->set_blendcol(RGBA(1,1,1,0.8));
  attach(leftarrow);*/
  
/*
  homebtn = new ReturnButton(this);
  homebtn->set_origin(Vec2(10,10));
  attach(homebtn);*/
  
  set_opacity_factor(0.0);  
  hide();
  disable();
}

EventReceiver* AppPanel::catches_event(const Event &ev) {
  EventReceiver *w = Widget::catches_event(ev);
  if(w)
    return w;
  return this;
}

/*EventReceiver* AppPanel::catches_event(const Event &ev) { 
  /*if(ev.get_event_type() <= EventTypeMouse)
    if(bg->get_shape().contains(dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin()))
      return this;
  return NULL;
  if(toggle_state)
    return this;
  return NULL;
}*/

void AppPanel::attach(Widget *w) {
  w->set_color_scheme(base, darkbase);
  Widget::attach(w);
}

void AppPanel::attach(Widget *w, bool invscheme) {
  if(invscheme) {
    w->set_color_scheme(darkbase, base);
    Widget::attach(w);
  } else {
    attach(w);
  }
}

void AppPanel::show_panel() {
  if(toggle_state)
    return;
  
  show();
  blend_state = -2.0;
  
  toggle_state = true;
  Renderable::context->register_animator(this);
  
  on_panel_show();
}

void AppPanel::hide_panel() {
  if(!toggle_state)
    return;
  
  disable();

  toggle_state = false;
  Renderable::context->register_animator(this);
  
  on_panel_hide();
}

void AppPanel::step() {
  if(toggle_state) {
    blend_state += 0.065;
    if(blend_state > 1.0) {
      blend_state = 1.0;
      unregister();
      enable();
    }
  }else {
    blend_state -= 0.065;
    if(blend_state < 0.0) {
      if(home)
        home->show_bar();
      unregister();
      hide();
    }
  }
  set_opacity_factor(std::max(0.0f, blend_state));
  set_origin(Vec2(1024 - blend_state * 1024, 0));
}

void AppPanel::set_home(WidgetNavigation *_home) {
  if(home)
    return;
  
  home = _home;
}

void AppPanel::on_panel_show() {}
void AppPanel::on_panel_hide() {}

//void AppPanel::on_mouse_click(const Vec2 &cursor) { }
