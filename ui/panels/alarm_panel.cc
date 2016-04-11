#include "alarm_panel.h"
#include "../widgets/panel.h"
#include "../widgets/timeselector.h"
#include "../geom.h"
#include "../../storage/storage.h"

#include <iostream>
#include <sstream>
#include <SDL/SDL.h>

#define ALARMFAV_BTN_H 63
#define LAUNCH_BUTTON_SIZE 75
#define TARGET_AMBIENT_VOLUME 0.07f

using namespace std;

AlarmPlayer::AlarmPlayer(EntertainmentPanel *_entertainment_panel)
: MediaPlayer(), entertainment_panel(_entertainment_panel) {
  //set_volume(0.7);
}

void AlarmPlayer::play_alarm() {
  if(get_opened_streams().size() != 0)
    return;
  
  float before;
  if(entertainment_panel != NULL) {
    before = entertainment_panel->player->get_volume();
    
    float v = before;
    while(v > TARGET_AMBIENT_VOLUME) {
      v -= 0.02f;
      entertainment_panel->player->set_volume(max(TARGET_AMBIENT_VOLUME,v));
      SDL_Delay(10);
    }
  }
  
  open(new MediaStream("data/sounds/alarm.wav"))->play();
  while(get_opened_streams().size() != 0) {
    SDL_Delay(100);
  }
  
  if(entertainment_panel != NULL) {
    while(entertainment_panel->player->get_volume() < before-0.00001) {
      float v = entertainment_panel->player->get_volume();
      entertainment_panel->player->set_volume(min(before,v + 0.02f));
      SDL_Delay(10);
    }
  }
  entertainment_panel->player->set_volume(before);
}


AlarmFavorite::AlarmFavorite(AlarmPanel *_parent, AlarmHistory *_hist, string time, int secs)
: Button(170, ALARMFAV_BTN_H, time),
  seconds(secs),
  parent(_parent),
  hist(_hist)
{
  set_color_scheme(RGBA("#ffffff",1),RGBA("#ffffff",1));
}
  
void AlarmFavorite::callback() {
  parent->time_selector->set_seconds(seconds);
  //hist->update_list();
}

AlarmHistory::AlarmHistory(AlarmPanel *_parent)
: pos(0),
  parent(_parent)
{
  bg = new Box(Rect(72+0,0,72+170,600));
  bg->set_background_color(RGBA("#ffffff",0.6));
  RenderableGroup::attach(bg);
}

EventReceiver* AlarmHistory::catches_event(const Event &ev) {
  return Widget::catches_event(ev);
}
    
void AlarmHistory::append_alarm(string time, int secs) { 
  Button *btn = new AlarmFavorite(parent, this, time, secs);
  btn->translate(Vec2(72, pos));
  pos = pos + ALARMFAV_BTN_H + 1;
  attach(btn);
     
  if(pos < 600)
    bg->set_shape(Rect(72+0,pos,72+170,600));
  else
    bg->hide();
}
    
void AlarmHistory::update_list() {
  for(int i = 0; i < get_children().size(); ++i)
    detach((Widget*)(get_children()[i]));
  
  get_children().clear();
  evrcv_children.clear();

  RenderableGroup::attach(bg);
  pos = 0;
  
  Storage::CmdResult res;
  Storage::instance().exec("SELECT seconds FROM timers ORDER BY occurrence DESC;", &res);
  
  stringstream tmp;
  
  int n = 0;
  for(auto x: res["seconds"]) {
    tmp.str(string());
    int secs = atoi(x.c_str());
    tmp << (secs / 60) << '\'' << (secs % 60) << '"';
    append_alarm(tmp.str(), secs);
    
    n++;
    if(n == 9)
      break;
  }
  
  bg->set_shape(Rect(72+0,pos,72+170,600));
    
  Renderable::context->refresh();
}

class LaunchButton: public Button {
  AlarmPanel *panel;
  Img *notif_img;
public:
  LaunchButton(AlarmPanel *_panel, int pos, Img* i)
  : Button(LAUNCH_BUTTON_SIZE,LAUNCH_BUTTON_SIZE,i),
    panel(_panel),
    notif_img(i)
  {
    translate(Vec2(72+170+32+pos * (LAUNCH_BUTTON_SIZE+2),600-LAUNCH_BUTTON_SIZE-32));
  }
  
  void callback() {
    if(panel->time_selector->get_seconds() == 0)
      return;
    
    int secs = panel->time_selector->get_seconds();
    panel->launch_alarm(secs, notif_img);
    
    //Storage::instance().cmd() << "INSERT INTO timers VALUES (" << secs << ");";
    
    Storage::instance().cmd() <<
      "INSERT INTO timers(seconds)" <<
      " SELECT " << secs << " WHERE NOT EXISTS(SELECT 1 FROM timers WHERE seconds=" << secs << ");" <<
      "UPDATE timers SET occurrence=occurrence+1 WHERE seconds=" << secs << ";";
    Storage::instance().exec();
    
    panel->alarm_history->update_list();
  }
};

AlarmPanel::AlarmPanel(EntertainmentPanel *_entertainment_panel)
: AppPanel(RGBA("#1abc9c", 1), RGBA("#16a085", 1)),
  entertainment_panel(_entertainment_panel),
  aplayer(_entertainment_panel)
{
  Storage::instance().exec("CREATE TABLE IF NOT EXISTS timers (seconds INT UNIQUE, occurrence INT DEFAULT 0);");

  Storage::CmdResult res;
  Storage::instance().exec("SELECT seconds FROM timers ORDER BY occurrence DESC;", &res);
  
  alarm_history = new AlarmHistory(this);
  stringstream tmp;
  int n = 0;
  for(auto x: res["seconds"]) {
    tmp.str(string());
    int secs = atoi(x.c_str());
    tmp << (secs / 60) << '\'' << (secs % 60) << '"';
    alarm_history->append_alarm(tmp.str(), secs);
    
    n++;
    if(n == 9)
      break;
  }
  attach(alarm_history);
  
  /*ah->append_alarm("3min", 3, 0);
  ah->append_alarm("20min", 20, 0);
  ah->append_alarm("15min 59secs", 15, 0);*/
  
  Rect r(0,0,32,32);
  attach(new LaunchButton(this, 0, new Img("data/widgets/pot.png", r)));
  attach(new LaunchButton(this, 1, new Img("data/widgets/pan.png", r)));
  attach(new LaunchButton(this, 2, new Img("data/widgets/oven.png", r)));
  attach(new LaunchButton(this, 3, new Img("data/widgets/egg.png", r)));
  attach(new LaunchButton(this, 4, new Img("data/widgets/tea.png", r)));
  attach(new LaunchButton(this, 5, new Img("data/widgets/wait.png", r)));

  time_selector = new TimeSelector(Vec2(72+170+32+(LAUNCH_BUTTON_SIZE+2)*3, 240));
  attach(time_selector);
  
  lst = new List(Rect(1024-250,0,1024,600));
  attach(lst);
}

AlarmPanel::~AlarmPanel() {}

AlarmPanel::AlarmRunningItem::AlarmRunningItem(std::string s, AlarmData *_ad, Img *i)
: ListItemText(s, ALARMFAV_BTN_H),
  ad(_ad)
{}

void AlarmPanel::AlarmRunningItem::on_removed() {
  ad->abort = true;
}

int AlarmPanel::blinking_icon(void *_ad) {
  AlarmData *ad = (AlarmData*)_ad;
  
  cout << "sdlkfn" << endl;
  
  for(int i = 0; i < 50 && ad->self->hidden() == false; ++i) {
    ad->img->set_blendcol(RGBA(0.3f * (1 - cos(i*(8*M_PI)/50.f)),0,0,0.6f));
    Renderable::context->refresh();
    SDL_Delay(80);
  }
  return 0;
}

int AlarmPanel::alarm_function(void *_ad) {
  AlarmData *ad = (AlarmData*)_ad;
  
  /*((AlarmData*)_ad)->done = true;*/
  
  int rem_secs;
  for(int i = 0; i <= ad->secs && ad->abort == false; ++i) {
    SDL_Delay(1000);
    rem_secs = ad->secs - i;
    
    std::stringstream s;
    s << "T-" << (rem_secs / 60) << "'" << (rem_secs % 60) << '"';
    ad->list_item->set_text(s.str());
    
    if(ad->self->get_toggle_state())
      Renderable::context->refresh();
  }
  
  if(ad->abort == false) {
    SDL_Thread *t = SDL_CreateThread(&blinking_icon, _ad);
    ad->self->aplayer.play_alarm();
    ad->self->lst->remove_item(ad->list_item);
    SDL_WaitThread(t, NULL);
  }else
    cout << "Alarm " << ad->secs << " aborted" << endl;
  
  delete ad;
  return 0;
}

void AlarmPanel::launch_alarm(int secs, Img *img) {
  if(secs < 0)
    return;
    
  Rect r(203,16,235,48);

  AlarmData *ad = new AlarmData();
  ad->self = this;
  ad->secs = secs;
  ad->img = new Img(img->get_source(), r);
  ad->abort = false;
  
  std::stringstream s;
  s << "T-" << (secs / 60) << "'" << (secs % 60) << '"';
  ad->list_item = new AlarmRunningItem(s.str(), ad, ad->img);
  
  SDL_CreateThread(&AlarmPanel::alarm_function, (void*)ad);
  
  lst->append_item(ad->list_item);
  
  ad->img->set_blendcol(RGBA(0,0,0,0.6f));
  ad->list_item->get_container()->attach(ad->img);
}

