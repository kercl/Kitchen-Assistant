#include "mediaplayerui.h"
#include "../panels/entertainment_panel.h"
#include "slider.h"
#include <string>

using namespace std;

class PlayButton: public Button {
  EntertainmentPanel *basepanel;
public:
  PlayButton(EntertainmentPanel *_parent)
  : Button(72, 72, new Img("data/widgets/play.png", Rect(0,0,32,32))),
    basepanel(_parent)
  {
    image->set_blendcol(RGBA(1,1,1,0.9));
    set_color_scheme(RGBA("#ffffff",1),RGBA("#ffffff",1));
  }
  
  void callback() {
    basepanel->player->stop();
  }
};

class VolumeSlider: public Slider {
  EntertainmentPanel *basepanel;
  bool blocked;
public:
  VolumeSlider(EntertainmentPanel *_basepanel)
  : Slider(36,200,Slider::vertical_orientation,1.0f),
    basepanel(_basepanel), 
    blocked(false)
  {
    translate(Vec2(0,-200));
    set_invert(true);
    hide();
    disable();

    /*
    float vol;
    Storage::instance().get_setting("global_mediaplayer_volume", vol, 1.0f);
    cout << " -- Slider: reading snapshot -- " << vol << endl;
    //basepanel->player->set_volume(vol);
    set_value(vol, false);*/
  }
  
  void callback() {
    basepanel->player->set_volume(get_value());
    if(!blocked)
      basepanel->player->snapshot();
    blocked = true;
  }
  
  void on_drag_end(const MouseEvent &ev) {
    basepanel->player->snapshot();
    blocked = false;
  }
};

class VolumeButton: public Button {
  EntertainmentPanel *basepanel;
  VolumeSlider *sliderbox;
public:
  VolumeButton(EntertainmentPanel *_parent)
  : Button(36, 72, new Img("data/widgets/speaker.png", Rect(0,0,32,32))),
    basepanel(_parent)
  {
    image->set_blendcol(RGBA(1,1,1,0.9));
    set_color_scheme(RGBA("#ffffff",1),RGBA("#ffffff",1));
    
    sliderbox = new VolumeSlider(basepanel);
    sliderbox->set_color_scheme(basepanel->get_base_color_dark(), basepanel->get_base_color_dark());
    Widget::attach(sliderbox);
  }
  
  void callback() {
    if(sliderbox->hidden()) {
      sliderbox->show();
      sliderbox->enable();
    }else {
      sliderbox->hide();
      sliderbox->disable();
    }
    Renderable::context->refresh();
  }
  
  EventReceiver* catches_event(const Event &ev) {
    sliderbox->set_mouse_position_correction(get_origin());
    EventReceiver* ret = Button::catches_event(ev);
    if(ret == NULL) {
      MouseEvent tmp(dynamic_cast<const MouseEvent&>(ev));
      tmp.set_cursor(tmp.get_cursor() - get_origin());
      return Widget::catches_event(dynamic_cast<const MouseEvent&>(tmp));
    }
    return ret;
  }
  
  void step() {
    Button::step();
  }
  
  VolumeSlider *get_slider() { return sliderbox; }
};

class PlayBar: public Widget {
    EntertainmentPanel *_parent;
    Text *caption;
    VolumeButton *vbtn;
  public:
    PlayBar(EntertainmentPanel *parent) 
    : _parent(parent)
    {
      Box *bg = new Box(Rect(72+300+50+72+5,600-72-50,1024-50-36-5,600-72-50+72));
      bg->set_background_color(RGBA("#ffffff",0.6));
      RenderableGroup::attach(bg);
      
      PlayButton *pbtn = new PlayButton(parent);
      pbtn->translate(Vec2(72+300+50, 600-72-50));
      pbtn->set_color_scheme(parent->get_base_color_dark(), parent->get_base_color_dark());
      
      attach(pbtn);
      
      caption = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 13), 
                         Vec2(72+300+50+72+5+10, 600-72-50+36-7));
      caption->set_blendcol(RGBA(0,0,0,0.7));
      RenderableGroup::attach(caption);
      
      vbtn = new VolumeButton(parent);
      vbtn->translate(Vec2(1024-50-36, 600-72-50));
      vbtn->set_color_scheme(parent->get_base_color_dark(), parent->get_base_color_dark());
      attach(vbtn);
    }
    EventReceiver* catches_event(const Event &ev) {
      return Widget::catches_event(ev);
    }
    void render() {
      caption->get_stream().str(string());
      if(_parent->player->current_stream) {
        caption->get_stream() << _parent->player->current_stream->get_metadata("currentlyPlaying").c_str();
      }
      
      Widget::render();
    }
    VolumeSlider* get_volume_slider() { return vbtn->get_slider(); }
};

MediaPlayerUI::MediaPlayerUI(EntertainmentPanel *parent) {
  pb = new PlayBar(parent);
  attach(pb);
  current_stream = NULL;
}

void MediaPlayerUI::set_volume_from_settings() {
  float vol;
  Storage::instance().get_setting("global_mediaplayer_volume", vol, 1.0f);
  set_volume(vol);
  pb->get_volume_slider()->set_value(vol, false);
}

void MediaPlayerUI::render() {
  Widget::render();
}

FadingMediaStream::FadingMediaStream(const char *s): MediaStream(s) {
  locked = true;
}
  
void FadingMediaStream::on_stream_opened() {
  fade_in(1000);
    
  for(auto x: parent->get_opened_streams()) {
    if(x != this)
      x->fade_out(1000, true);
  }
  locked = false;
  
  on_update_metadata();
}
  
void FadingMediaStream::on_stream_open_failed() {
  locked = false;
}

void FadingMediaStream::on_update_metadata() {
  Renderable::context->refresh();
}

void MediaPlayerUI::snapshot() {
  Storage::instance().set_setting("global_mediaplayer_volume", get_volume());
}

void MediaPlayerUI::stop() {
  for(auto x: get_opened_streams()) {
    x->fade_out(1000, true);
  }
}

MediaPlayerUI::~MediaPlayerUI() {
  snapshot();
}

bool FadingMediaStream::locked = false;

