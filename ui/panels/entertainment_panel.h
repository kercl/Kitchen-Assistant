#ifndef ENTERTAINMENT_PANEL_H
#define ENTERTAINMENT_PANEL_H

#include "../widgets/panel.h"
#include "../widgets/mediaplayerui.h"

class EntertainmentPanel: public AppPanel {
public:
  EntertainmentPanel();
  ~EntertainmentPanel();
  
  RGBA get_base_color_dark();
  MediaPlayerUI *player;
  
  void on_panel_show();
};

#endif

