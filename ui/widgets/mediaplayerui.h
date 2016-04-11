#ifndef MEDIAPLAYERUI_H
#define MEDIAPLAYERUI_H

#include "button.h"
#include "../../mediaplayer/mediaplayer.h"
#include "../../storage/storage.h"

class EntertainmentPanel;
class PlayBar;

class FadingMediaStream: public MediaStream {
public:
  static bool locked;

  FadingMediaStream(const char *s);
  void on_stream_opened();
  void on_stream_open_failed();
  void on_update_metadata();
};

class MediaPlayerUI: public MediaPlayer, public Widget, public PersistentSettings {
  PlayBar *pb;
public:
  MediaPlayerUI(EntertainmentPanel *parent);
  ~MediaPlayerUI();
  void render();
  
  MediaStream *current_stream;
  
  void stop();
  
  void snapshot();
  void set_volume_from_settings();
};

#endif

