#include "navigation.h"
#include "notification.h"

#include "../animation.h"
#include "../box.h"
#include "../img.h"

#include <iostream>

using namespace std;

WidgetNavigationButtonAlarm::WidgetNavigationButtonAlarm(AppPanel *target_panel)
: WidgetNavigationButton("data/widgets/stopwatch.png", target_panel) {
  //bg->set_background_color(RGBA(1,1,1,highlight));
  bg->set_background_color(RGBA("#1abc9c",highlight));
}

WidgetNavigationButtonShoppingList::WidgetNavigationButtonShoppingList(AppPanel *target_panel)
: WidgetNavigationButton("data/widgets/shoppinglist.png", target_panel) {
  bg->set_background_color(RGBA("#e67e22",highlight));
}

WidgetNavigationButtonEntertainment::WidgetNavigationButtonEntertainment(AppPanel *target_panel)
: WidgetNavigationButton("data/widgets/entertainmen.png", target_panel) {
  bg->set_background_color(RGBA("#3498db",highlight));
}

WidgetNavigationButtonBrowser::WidgetNavigationButtonBrowser(AppPanel *target_panel)
: WidgetNavigationButton("data/widgets/globe.png", target_panel) {
  bg->set_background_color(RGBA("#2ecc71",highlight));
}

WidgetNavigationButtonSettings::WidgetNavigationButtonSettings(AppPanel *target_panel)
: WidgetNavigationButton("data/widgets/cogwheels.png", target_panel) {
  bg->set_background_color(RGBA("#8e44ad",highlight));
}

WidgetNavigationButton::WidgetNavigationButton(string imgpath, AppPanel *_target_panel): Animator(10) {
  highlight = 0.4;
  target_panel = _target_panel;
  
  Rect shape(0, 0, 80, 80);
  bg = new Box(shape);
  //bg->set_background_color(RGBA(1,1,1,highlight));
  //bg->drop_shadow(Vec2(0,0),4,RGBA(0,0,0,0.35));
  bg->set_border_radius(30);
  /*bg->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.6)},
    {100,RGBA(1,1,1,0.0)}}), 
    TexCoords(Vec2(0.2,0.2), Vec2(0.2,1.0), Vec2(1.0,1.0), Vec2(1.0,0.2)) );*/
  RenderableGroup::attach(bg);
  
  if(imgpath != "") {
    shape.left += 15;
    shape.right -= 15;
    shape.top += 15;
    shape.bottom -= 15;
    Img *img = new Img(imgpath, shape);
    img->set_blendcol(RGBA(1,1,1,0.8));
    RenderableGroup::attach(img);
  }
}

void WidgetNavigationButton::set_parent(WidgetNavigation *par) {
  parent = par;
  if(target_panel)
    target_panel->set_home(par);
}

void WidgetNavigationButton::on_mouse_down(const MouseEvent &ev) {
  highlight = 0.8;
  Renderable::context->register_animator(this);
}

void WidgetNavigationButton::on_mouse_click(const MouseEvent &ev) {
  if(target_panel) {
    target_panel->show_panel();
    if(parent)
      parent->hide_bar();
  }
}

void WidgetNavigationButton::step() {
  highlight -= 0.015;
  if(highlight < 0.4) {
    highlight = 0.4;
    unregister();
  }
  bg->set_background_alpha(highlight);
}

EventReceiver* WidgetNavigationButton::catches_event(const Event &ev) {
  if(ev.get_event_type() <= EventTypeMouse) {
    if(bg->get_shape().contains(dynamic_cast<const MouseEvent&>(ev).get_cursor() - get_origin()))
      return this;
  }
  return NULL;
}

void WidgetNavigation::step() {
  int t1 = 250,
      t2 = 400,
      delta = -70;
  float d = (t1*t1*t2-t1*t2*t2);
  float a = (300 * t1 + delta * t2)/d,
        b = -(300 * t1*t1 + t2*t2*delta)/d;
  set_origin(Vec2(a*t*t+b*t, 0));
  
  if(toggle_status == false) {
    t += elapsed;
    if(t >= t2) {
      unregister();
      hide();
      t = t2;
    }
  }else {
    t -= elapsed;
    if(t < 0) {
      set_origin(Vec2(0, 0));
      unregister();
      enable();
      t = 0;
    }
  }
}

WidgetNavigation::WidgetMask::WidgetMask(Widget *mof) {
  maskof = mof;
  Box *m = new Box(Rect(-30,0,110,600));
  m->set_background_color(RGBA(1,1,1,1));
  attach(m);
}

void WidgetNavigation::WidgetMask::render() {
  set_angle(maskof->get_angle());
  set_origin(maskof->get_origin());
  RenderableGroup::render();
}

Renderable* WidgetNavigation::get_mask() {
  return &mask;
}

WidgetNavigation::WidgetNavigation(WidgetNotificationPanel *wnp, Wallpaper *wp, const vector<WidgetNavigationButton*> &panel_assignments)
: toggle_status(true), 
  mask(this), 
  notif_panel(wnp),
  wallp(wp),
  t(0), 
  Animator(15),
  shape(-30,0,110,600)
{
  Box *b = new Box(shape);
  b->set_background_color(RGBA(0,0,0,0.4));/*
  b->drop_shadow(Vec2(2,0),10,RGBA(0,0,0,0.5));
  b->append_texture(new LinearGradientTexture({
      {0, RGBA(0,0,0,0.1)},
      {15,RGBA(0,0,0,0.15)},
      {20,RGBA(0,0,0,0.4)},
      {22,RGBA(0,0,0,0.6)},
      {24,RGBA(0,0,0,0.8)}}),
      TexCoords(Vec2(0,0), Vec2(0,1), Vec2(1,1), Vec2(1,0)) );*/
      /*
  b->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {100,RGBA(1,1,1,0.0)}}),
    TexCoords(Vec2(0.2,0.2), Vec2(0.9,0.2), Vec2(0.9,2.8), Vec2(0.2,2.8)) );
  b->append_texture(new RadialGradientTexture({
    {0,RGBA(1,1,1,0.5)},
    {100,RGBA(1,1,1,0.0)}}),
    TexCoords(Vec2(0.1,-1.0), Vec2(0.7,-1.0), Vec2(0.7,0.7), Vec2(0.1,0.7)) );*/
  RenderableGroup::attach(b);
  
  Widget *buttonlist = new Widget();
  
  int pos = 0;
  for(auto x: panel_assignments) {
    if(x) {
      x->set_origin(Vec2(15, (600 - panel_assignments.size() * (80 + 15) - 15)/2 + (80 + 15) * pos));
      x->set_parent(this);
      buttonlist->attach(x);
      pos++;
    }
  }
  
  attach(buttonlist);
}

void WidgetNavigation::show_bar() {
  if(toggle_status)
    return;
    
  disable(); // let animations finish
  
  Renderable::context->register_animator(wallp);

  toggle_status = true;
  show();
  Renderable::context->register_animator(this);
  if(notif_panel)
    notif_panel->show_panel();
}

void WidgetNavigation::hide_bar() {
  if(!toggle_status)
    return;
  
  disable(); // let animations finish
  
  //wallp->unregister();

  toggle_status = false;
  Renderable::context->register_animator(this);
  if(notif_panel)
    notif_panel->hide_panel();
}

void WidgetNavigation::toggle_bar() {
  toggle_status = !toggle_status;
}

