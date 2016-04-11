#include "entertainment_panel.h"
#include "../widgets/panel.h"
#include "../img.h"

#include <iostream>
#include <string>
#include <SDL/SDL.h>

#define RADIO_MODE 0
#define TV_MODE 1

#define SELECTOR_BTN_H 63

using namespace std;

class StationButton: public Button {
  string source;
  EntertainmentPanel *parent;
public:
  StationButton(EntertainmentPanel *_parent, string station, string _source)
  : Button(300, SELECTOR_BTN_H, station),
    source(_source),
    parent(_parent)
  {
    set_color_scheme(RGBA("#ffffff",1),RGBA("#ffffff",1));
  }
  
  void callback() {
    if(FadingMediaStream::locked == false) {
      parent->player->current_stream = parent->player->open(new FadingMediaStream(source.c_str()));
    }
  }
};

class StationSelector: public Widget {
    int mode;
    int pos;
    Box *bg;
    EntertainmentPanel *parent;
  public:
    StationSelector(EntertainmentPanel *_parent)
    : mode(RADIO_MODE), 
      pos(0),
      parent(_parent)
    {
      bg = new Box(Rect(72+0,0,72+300,600));
      bg->set_background_color(RGBA("#ffffff",0.6));
      RenderableGroup::attach(bg);
    }
    EventReceiver* catches_event(const Event &ev) {
      return Widget::catches_event(ev);
    }
    
    void append_station(int mode, string station, string source) { 
      Button *btn = new StationButton(parent, station, source);
      btn->translate(Vec2(72, pos));
      pos = pos + SELECTOR_BTN_H + 1;
      attach(btn);
      
      if(pos < 600)
        bg->set_shape(Rect(72+0,pos,72+300,600));
      else
        bg->hide();
    }
};

class RadioButton: public Button {
  public:
    RadioButton(Img *i): Button(72, 72, i) {
      translate(Vec2(0,72));
    }
    
    void callback() {
      cout << "select radio mode" << endl;
    }
};

class TVButton: public Button {
  public:
    TVButton(Img *i): Button(72, 72, i) {
      translate(Vec2(0,72 * 2));
    }
    
    void callback() {
      cout << "select TV mode" << endl;
    }
};

RGBA EntertainmentPanel::get_base_color_dark() {
  return darkbase;
}

EntertainmentPanel::EntertainmentPanel(): AppPanel(RGBA("#3498db", 1), RGBA("#2980b9", 1)) {
  Img *radio = new Img("data/widgets/radio.png", Rect(0,0,32,32));
  radio->set_blendcol(RGBA(1,1,1,0.8));
  Button *radio_btn = new RadioButton(radio);
  
  Img *tv = new Img("data/widgets/screen.png", Rect(0,0,32,32));
  tv->set_blendcol(RGBA(1,1,1,0.8));
  Button *tv_btn = new TVButton(tv);
  
  StationSelector *ss = new StationSelector(this);
  attach(ss);
  
  ss->append_station(RADIO_MODE, "FM4", "http://mp3stream1.apasf.apa.at:8000/");
  ss->append_station(RADIO_MODE, "Ministry of Sound Radio", "rtmp://wowza05.sharp-stream.com/mosaac/mosaac");
  ss->append_station(RADIO_MODE, "Kool FM", "http://w10.streamgb.com:1935/kool/kool/playlist.m3u8");
  ss->append_station(RADIO_MODE, "BBC Radio 1", "http://bbcmedia.ic.llnwd.net/stream/bbcmedia_radio1_mf_p");
  ss->append_station(RADIO_MODE, "BBC Radio 1 Xtra", "http://bbcmedia.ic.llnwd.net/stream/bbcmedia_radio1xtra_mf_p");
  ss->append_station(RADIO_MODE, "BBC World Service", "http://bbcwssc.ic.llnwd.net/stream/bbcwssc_mp1_ws-eieuk");
  //ss->append_station(RADIO_MODE, "Bismuth", "");
  
  Img *i = new Img("data/widgets/quaver.png", Rect(72+300+200, 70, 1024-150, 600-250));
  i->set_blendcol(RGBA(1,1,1,0.03));
  RenderableGroup::attach(i);
  
  player = new MediaPlayerUI(this);
  attach(player);
  
  attach(radio_btn);
  attach(tv_btn);
  
  player->set_volume_from_settings();
}

void EntertainmentPanel::on_panel_show() {}

EntertainmentPanel::~EntertainmentPanel() {
  
}

