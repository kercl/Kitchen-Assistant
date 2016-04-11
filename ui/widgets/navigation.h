#ifndef WIDGET_NAVIGATION_H
#define WIDGET_NAVIGATION_H

#include <vector>

#include "widget.h"
#include "panel.h"
#include "clock.h"
#include "../texture.h"
#include "../render.h"
#include "../box.h"
#include "../img.h"

class WidgetNavigation;
class WidgetNotificationPanel;
class AppPanel;

class WidgetNavigationButton: public Widget, public Animator {
  protected:
    WidgetNavigation *parent;
    AppPanel *target_panel;
    Box *bg;
    float highlight;
  public:
    WidgetNavigationButton(std::string imgpath, AppPanel *target_panel);
    
    void set_parent(WidgetNavigation *par);
    
    void on_mouse_down(const MouseEvent &ev);
    void on_mouse_click(const MouseEvent &ev);
    EventReceiver* catches_event(const Event &ev);
    
    void step();
};

class WidgetNavigationButtonAlarm: public WidgetNavigationButton {
public:
  WidgetNavigationButtonAlarm(AppPanel *target_panel);
  void callback();
};

class WidgetNavigationButtonShoppingList: public WidgetNavigationButton {
public:
  WidgetNavigationButtonShoppingList(AppPanel *target_panel);
  void callback();
};

class WidgetNavigationButtonEntertainment: public WidgetNavigationButton {
public:
  WidgetNavigationButtonEntertainment(AppPanel *target_panel);
  void callback();
};

class WidgetNavigationButtonBrowser: public WidgetNavigationButton {
public:
  WidgetNavigationButtonBrowser(AppPanel *target_panel);
  void callback();
};

class WidgetNavigationButtonSettings: public WidgetNavigationButton {
public:
  WidgetNavigationButtonSettings(AppPanel *target_panel);
  void callback();
};

class WidgetNavigation: public Widget, public Animator {
    bool toggle_status;
    
    WidgetNotificationPanel *notif_panel;
    Wallpaper *wallp;
    
    class WidgetMask: public RenderableGroup {
      Widget *maskof;
    public:
      WidgetMask(Widget *maskof);
      void render();
    };
    WidgetMask mask;
    int t;
    Rect shape;
  public:
    WidgetNavigation(WidgetNotificationPanel *wnp, Wallpaper *wp, const std::vector<WidgetNavigationButton*> &panel_assignments);
    
    void show_bar();
    void hide_bar();
    void toggle_bar();
  
    void step();
    
    Renderable* get_mask();
};

#endif

