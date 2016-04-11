#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
}

#include <vector>
#include <SDL/SDL_thread.h>
#include <string>
#include <deque>
#include <map>

#define MINIMAL_PACKET_QUEUE_LEN 10

#define STREAM_UNKNOWN -1
#define STREAM_FM4 0
#define STREAM_BBCRADIO1 1

class MediaPlayer;

class MediaStream {
protected:
  bool _opened, _playing;
  AVFormatContext *fctx;
  int videoStream, audioStream;
  float volume_factor;
  
  static int audio_stream_thread(void *ptr);
  static int open_streams(void *ptr);
  
  std::vector<SDL_Thread*> threads;
  MediaPlayer *parent;
  
  std::string stream_source;
  int internal_stream_id;
  
  std::map<std::string,std::string> metadata;
public:
  MediaStream(MediaPlayer *_parent, const char *source);
  MediaStream(const char *source);
  ~MediaStream();
  
  void set_parent(MediaPlayer *_parent);
  
  void play();
  void pause();
  virtual void stop(bool dismiss = false);
  
  void fade_out(int time, bool dismiss = false);
  void fade_in(int time);
  
  bool is_open();
  
  void set_volume(float factor);
  std::string get_av_metadata(std::string key);
  std::string get_metadata(std::string key);
  
  virtual void on_stream_opened();
  virtual void on_stream_open_failed();
  virtual void on_update_metadata();
};

class MediaPlayer {
  static bool registered;
  static int openedPlayers;
  std::deque<MediaStream*> openedstreams, dismissedstreams;
  float global_volume;
 
  static int driver;
  
  friend class MediaStream;
  
  void cleanup();
public:
  MediaPlayer();
  ~MediaPlayer();
  
  void dismiss_stream(MediaStream *strm);
  
  MediaStream* open(const char *src);
  MediaStream* open(MediaStream* srcstrm);
  
  std::deque<MediaStream*>& get_opened_streams();
  
  void set_volume(float v);
  float get_volume() const;
  
  static void shutdown();
};

#endif

