#ifndef APP_H
#define APP_H

#include "../animation.h"
#include "event.h"
#include "navigation.h"
#include "widget.h"
#include "button.h"
#include "../box.h"

class WidgetNavigation;
class AppPanel;

class AppPanel: public Widget, public Animator {
  float blend_state;
  bool toggle_state;
  WidgetNavigation *home;
  Box *bg;
protected:
  RGBA base, darkbase;
public:
  AppPanel(RGBA base, RGBA darkbase);
  virtual void step();
  
  EventReceiver* catches_event(const Event &ev);

  void show_panel();
  void hide_panel();
  
  bool get_toggle_state() { return toggle_state; }
  
  void set_home(WidgetNavigation *home);
  
  virtual void attach(Widget *w);
  virtual void attach(Widget *w, bool invscheme);
  
  virtual void on_panel_show();
  virtual void on_panel_hide();
  
  Box* get_background();
};

#endif

