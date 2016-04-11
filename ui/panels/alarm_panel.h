#ifndef ALARM_PANEL_H
#define ALARM_PANEL_H

#include "../widgets/panel.h"
#include "../widgets/list.h"
#include "../img.h"
#include "entertainment_panel.h"
#include "../widgets/timeselector.h"
#include "../../mediaplayer/mediaplayer.h"

#include <string>
#include <iostream>

class AlarmPanel;
class AlarmHistory;

class AlarmPlayer: public MediaPlayer {
  EntertainmentPanel *entertainment_panel;
public:
  AlarmPlayer(EntertainmentPanel *entertainment_panel);
  void play_alarm();
};

class AlarmFavorite: public Button {
  std::string source;
  AlarmPanel *parent;
  AlarmHistory *hist;
  int seconds;
public:
  AlarmFavorite(AlarmPanel *_parent, AlarmHistory *_hist, std::string time, int secs);
  void callback();
};

class AlarmHistory: public Widget {
    int pos;
    Box *bg;
    AlarmPanel *parent;
  public:
    AlarmHistory(AlarmPanel *_parent);
    EventReceiver* catches_event(const Event &ev);
    void append_alarm(std::string time, int secs);
    void update_list();
};

class AlarmPanel: public AppPanel {
  EntertainmentPanel *entertainment_panel;
  AlarmPlayer aplayer;
  List *lst;
  
  class AlarmRunningItem;
  
  struct AlarmData {
    int secs;
    AlarmPanel *self;
    Img *img;
    bool abort;
    AlarmRunningItem *list_item;
  };
  
  class AlarmRunningItem: public ListItemText {
    AlarmData *ad;
  public:
    AlarmRunningItem(std::string s, AlarmData *_ad, Img *i);
    void on_removed();
  };
  
  static int blinking_icon(void *self);
  static int alarm_function(void *self);
  
  friend class AlarmRunningItem;
public:
  TimeSelector *time_selector;
  AlarmHistory *alarm_history;

  AlarmPanel(EntertainmentPanel *entertainment_panel = NULL);
  virtual ~AlarmPanel();
  
  void launch_alarm(int secs, Img *img);
};

#endif

