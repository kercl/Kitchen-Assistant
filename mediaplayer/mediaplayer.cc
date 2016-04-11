#include "mediaplayer.h"
#include "../storage/storage.h"

#include <iostream>
#include <limits>
#include <SDL/SDL_timer.h>
#include <ao/ao.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

using namespace std;

void MediaStream::set_volume(float factor) {
  if(factor > 1.0f)
    factor = 1.0f;
  if(factor < 0.0f)
    factor = 0.0f;
  volume_factor = factor;
}

void MediaStream::on_stream_opened() {}
void MediaStream::on_stream_open_failed() {}
void MediaStream::on_update_metadata() {}

std::string MediaStream::get_av_metadata(std::string key) {
  AVDictionaryEntry *tag = av_dict_get(fctx->metadata, key.c_str(), tag, AV_DICT_IGNORE_SUFFIX);
  if(!tag)
    return "";
  return string((char*)tag->value);
}

std::string MediaStream::get_metadata(std::string key) {
  return metadata[key];
}

int MediaStream::audio_stream_thread(void *ptr) {
  MediaStream *self = (MediaStream*)ptr;

  AVCodecContext *ctx = self->fctx->streams[self->audioStream]->codec;
  AVCodec *codec = avcodec_find_decoder(ctx->codec_id);

  if(codec==NULL)
    return 0;

  if(avcodec_open2(ctx,codec,NULL) < 0)
    return 0;

  ao_sample_format sformat;
  AVSampleFormat sfmt=ctx->sample_fmt;
  if(sfmt==AV_SAMPLE_FMT_U8 || sfmt==AV_SAMPLE_FMT_U8P){
    sformat.bits=16;
  }else if(sfmt==AV_SAMPLE_FMT_S16 || sfmt==AV_SAMPLE_FMT_S16P){
    sformat.bits=16;
  }else if(sfmt==AV_SAMPLE_FMT_S32 || sfmt==AV_SAMPLE_FMT_S32P){
    sformat.bits=16;
  }else if(sfmt==AV_SAMPLE_FMT_FLT || sfmt==AV_SAMPLE_FMT_FLTP) {
    sformat.bits=16;
  }else if(sfmt==AV_SAMPLE_FMT_DBL || sfmt==AV_SAMPLE_FMT_DBLP) {
    sformat.bits=16;
  }else {
    // unsupported format
  }

  sformat.channels=ctx->channels;
  sformat.rate=ctx->sample_rate;
  sformat.byte_format=AO_FMT_NATIVE;
  sformat.matrix=0;

  ao_device *adevice = ao_open_live(MediaPlayer::driver, &sformat, NULL);
  
  AVPacket packet;
  av_init_packet(&packet);

  AVFrame *frame = av_frame_alloc();

  int buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE;

  uint8_t *buffer = new uint8_t[buffer_size];
  packet.data = buffer;
  packet.size = buffer_size;

  uint8_t *samples = new uint8_t[buffer_size];
  int len;
  int frameFinished = 0;

  int plane_size;
  
  self->on_update_metadata();

  while(self->_opened) {
    if(av_read_frame(self->fctx,&packet) >= 0 && self->_playing) {
      if(packet.stream_index == self->audioStream) {
        int len=avcodec_decode_audio4(ctx,frame,&frameFinished,&packet);
        int data_size = av_samples_get_buffer_size(&plane_size, ctx->channels, frame->nb_samples, ctx->sample_fmt, 1);
        uint16_t *out = (uint16_t *)samples;

        if(frameFinished){
          int write_p=0;

          switch (sfmt){
            case AV_SAMPLE_FMT_S16P:
              for (int nb = 0; nb<plane_size/sizeof(uint16_t); nb++) {
                for (int ch = 0; ch < ctx->channels; ch++) {
                  int16_t data = (int16_t)(((int16_t *) frame->extended_data[ch])[nb]);
                  data = (int16_t)(data * self->volume_factor * self->parent->global_volume);
                  out[write_p] = data;
                  write_p++;
                 }
              }
              ao_play(adevice, (char*)samples, (plane_size) * ctx->channels);
              break;
            case AV_SAMPLE_FMT_FLTP:
              for (int nb=0;nb<plane_size/sizeof(float);nb++){
                for (int ch = 0; ch < ctx->channels; ch++) {
                  out[write_p] = ((float *) frame->extended_data[ch])[nb] * std::numeric_limits<short>::max() * self->volume_factor * self->parent->global_volume ;
                  write_p++;
                }
              }
              ao_play(adevice, (char*)samples, ( plane_size/sizeof(float) )  * sizeof(uint16_t) * ctx->channels );
              break;
            case AV_SAMPLE_FMT_S16:
              ao_play(adevice, (char*)frame->extended_data[0],frame->linesize[0] );
              break;
            case AV_SAMPLE_FMT_FLT:
              for (int nb=0;nb<plane_size/sizeof(float);nb++){
                out[nb] = static_cast<short> ( ((float *) frame->extended_data[0])[nb] * std::numeric_limits<short>::max()  * self->volume_factor * self->parent->global_volume);
              }
              ao_play(adevice, (char*)samples, ( plane_size/sizeof(float) )  * sizeof(uint16_t) );
              break;
            case AV_SAMPLE_FMT_U8P:
              for (int nb=0;nb<plane_size/sizeof(uint8_t);nb++){
                for (int ch = 0; ch < ctx->channels; ch++) {
                  out[write_p] = ( ((uint8_t *) frame->extended_data[0])[nb] - 127) * std::numeric_limits<short>::max() * self->volume_factor * self->parent->global_volume / 127 ;
                  write_p++;
                }
              }
              ao_play(adevice, (char*)samples, ( plane_size/sizeof(uint8_t) )  * sizeof(uint16_t) * ctx->channels );
              break;
            case AV_SAMPLE_FMT_U8:
              for (int nb=0;nb<plane_size/sizeof(uint8_t);nb++){
                out[nb] = static_cast<short> ( ( ((uint8_t *) frame->extended_data[0])[nb] - 127) * std::numeric_limits<short>::max() * self->volume_factor * self->parent->global_volume / 127 );
              }
              ao_play(adevice, (char*)samples, ( plane_size/sizeof(uint8_t) )  * sizeof(uint16_t) );
              break;
            default:
              // pcm not supported
              break;
          }
        }
      }
      av_free_packet(&packet);
    }else {
      break;
    }
    
    if(self->internal_stream_id == STREAM_FM4) {
      std::string n = self->get_av_metadata("StreamTitle");
      if(n != self->metadata["currentlyPlaying"]) {
        self->metadata["currentlyPlaying"] = n;
        self->on_update_metadata();
      }
    }
    
    SDL_Delay(10);
  }

#ifdef DEBUG
  cerr << "closing stream " << self << endl;
#endif
  
  avcodec_close(ctx);
  delete buffer;
  delete samples;
  ao_close(adevice);
  
  self->stop(true); // stop and dismiss
  
  return 0;
}

void MediaStream::play() {
  _playing = true;
}

void MediaStream::pause() {
  _playing = false;
}

void MediaStream::stop(bool dismiss) {
  _playing = false;
  if(dismiss)
    parent->dismiss_stream(this);
}
  
void MediaStream::fade_out(int time, bool dismiss) {
  if(_playing == false)
    return;
  
  int deltat = time / 10;
  float initial_volume = volume_factor;
  
  for(int i = 0; i < time; i += 10) {
    set_volume(1.0f - ((float)i / (float)time) * initial_volume);
    SDL_Delay(10);
  }
  
  /*if(dismiss)
    parent->dismiss_stream(this);*/
  stop(dismiss);
}

void MediaStream::fade_in(int time) {
  if(_playing == true)
    return;
  
  _playing = true;
  
  int deltat = time / 10;
  float initial_volume = volume_factor;
  volume_factor = 0.0f;
  
  for(int i = 0; i < time; i += 10) {
    set_volume(((float)i / (float)time) * initial_volume);
    SDL_Delay(10);
  }
  set_volume(initial_volume);
}

MediaStream::MediaStream(const char *source): MediaStream(NULL, source) {}

int MediaStream::open_streams(void *ptr) {
  MediaStream *self = (MediaStream*)ptr;
  
  self->fctx = avformat_alloc_context();
  self->fctx->flags |= AVFMT_FLAG_IGNIDX; 

  if(avformat_open_input(&self->fctx, self->stream_source.c_str(), NULL, NULL) < 0) {
    self->on_stream_open_failed();
    self->internal_stream_id = STREAM_UNKNOWN;
    self->parent->dismiss_stream(self);
    return 0;
  }
  
  if(avformat_find_stream_info(self->fctx, NULL) < 0) {
    self->on_stream_open_failed();
    self->internal_stream_id = STREAM_UNKNOWN;
    self->parent->dismiss_stream(self); // possible problem with delete in dismiss_stream
    return 0;
  }

#ifdef DEBUG
  av_dump_format(self->fctx, 0, self->stream_source.c_str(), 0);
#endif

  for(int i=0;i < self->fctx->nb_streams;i++) {
    if(self->fctx->streams[i]->codec->codec_type ==AVMEDIA_TYPE_AUDIO 
        && self->audioStream < 0)
      self->audioStream = i;
    else if(self->fctx->streams[i]->codec->codec_type ==AVMEDIA_TYPE_VIDEO 
        && self->videoStream < 0)
      self->videoStream = i;
  }
  
  if(self->audioStream >= 0) {
    SDL_Thread *t = SDL_CreateThread(&MediaStream::audio_stream_thread, (void*)self);
    self->threads.push_back(t);
  }
  
  self->_opened = true;
  self->on_stream_opened();
}

MediaStream::MediaStream(MediaPlayer *_parent, const char *source)
: _opened(false), 
  _playing(false),
  fctx(NULL),
  videoStream(-1),
  audioStream(-1),
  volume_factor(1.0f),
  parent(_parent),
  stream_source(source)
{
  internal_stream_id = STREAM_UNKNOWN;
  if(!strcmp(source, "http://mp3stream1.apasf.apa.at:8000/"))
    internal_stream_id = STREAM_FM4;

  SDL_CreateThread(&MediaStream::open_streams, (void*)this);
}
  
bool MediaStream::is_open() {
  return _opened;
}

void MediaStream::set_parent(MediaPlayer *_parent) {
  parent = _parent;
}

MediaStream::~MediaStream() {
  _opened = false;
  _playing = false;
  
  int thread_ret;
  for(auto x: threads) {
    SDL_WaitThread(x, &thread_ret);
  }
  avformat_close_input(&fctx);
}

bool MediaPlayer::registered = false;
int MediaPlayer::openedPlayers = 0;
int MediaPlayer::driver = -1;

MediaPlayer::MediaPlayer() 
: global_volume(1.0f)
{
  if(!registered) {
    av_register_all();
    avformat_network_init();
    ao_initialize();
    driver = ao_default_driver_id();
    registered = true;
  }
  openedPlayers++;
}

void MediaPlayer::cleanup() {
  for(auto x: dismissedstreams)
    delete x;
  dismissedstreams.clear();
}

MediaStream* MediaPlayer::open(const char *src) {
  MediaStream *newstream = new MediaStream(this, src);
  if(newstream->is_open()) {
    openedstreams.push_back(newstream);
    return newstream;
  }
  
  cleanup();
  return NULL;
}

MediaStream* MediaPlayer::open(MediaStream* srcstrm) {
  openedstreams.push_back(srcstrm);
  srcstrm->set_parent(this);
  
  cleanup();
  return srcstrm;
}

void MediaPlayer::dismiss_stream(MediaStream *strm) {
  std::deque<MediaStream*>::iterator it = openedstreams.begin();
  while(it != openedstreams.end()) {
    if(*it == strm) {
      dismissedstreams.push_back(strm);
      openedstreams.erase(it);
      break;
    }
    it++;
  }
}

std::deque<MediaStream*>& MediaPlayer::get_opened_streams() {
  return openedstreams;
}

MediaPlayer::~MediaPlayer() {
  cleanup();
  for(auto x: openedstreams)
    delete x;
  
  openedPlayers--;
  if(openedPlayers == 0) {
    ao_shutdown();
    registered = false;
  }
}

void MediaPlayer::shutdown() {
  ao_shutdown();
}

void MediaPlayer::set_volume(float factor) {
  if(factor > 1.0f)
    factor = 1.0f;
  if(factor < 0.0f)
    factor = 0.0f;
  global_volume = factor;
}

float MediaPlayer::get_volume() const {
  return global_volume;
}

