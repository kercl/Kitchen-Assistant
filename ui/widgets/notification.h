#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "navigation.h"
#include "widget.h"
#include "clock.h"
#include "../texture.h"
#include "../render.h"
#include "../box.h"

class TraySettings: public Widget {
public:
  TraySettings();
};

class WidgetNotificationPanel: public Widget, public Animator {
    class WidgetMask: public RenderableGroup {
      Widget *maskof;
    public:
      WidgetMask(Widget *maskof);
      void render();
    };
    WidgetMask mask;
    
    bool toggle_state;
    float anim_r, anim_t, anim_a;
    
    ClockWidget *c;
    Text *date, *day;
    
    class WeatherUpdater: public Animator {
      WidgetNotificationPanel *panel;
      Img *weatherimages[4], *loadingimage;
      Text *temperature;
      int weathericon;
    public:
      WeatherUpdater(WidgetNotificationPanel *wnp);
      ~WeatherUpdater();
      void init();
      void step();
      
      static int weather_fetch_data(void *ptr);
    };
    WeatherUpdater weatherup;
    
    RenderableGroup *general_information;
    
    friend class WeatherUpdater;
  public:
    WidgetNotificationPanel();
    
    void render();
    
    void hide_panel();
    void show_panel();
    
    void step();
    
    Renderable* get_mask();
};

#endif

