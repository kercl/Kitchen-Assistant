#include "notification.h"
#include "../text.h"
#include "../../network/json11.h"

#include <iostream>
#include <curl/curl.h>
#include <SDL/SDL.h>

using namespace std;
using namespace json11;

void WidgetNotificationPanel::step() {
  if(toggle_state == false) {
    //anim_r += 0.03;
    anim_t -= 0.1;
    anim_a = anim_a - 0.06; //1.0 - anim_r * 2.0;
    
    if(anim_a < 0.0) {
      anim_a = 0.0;
      unregister();
      hide();
    }
  }else {
    anim_a += 0.05;
    
    if(anim_a > 1.0) {
      anim_a = 1.0;
      unregister();
    }
  }
}

WidgetNotificationPanel::WidgetMask::WidgetMask(Widget *mof) {
  maskof = mof;
  Box *m = new Box(Rect(235,70,954,520));
  m->set_background_color(RGBA(1,1,1,1));
  m->set_border_radius(5);
  attach(m);
}

void WidgetNotificationPanel::WidgetMask::render() {
  WidgetNotificationPanel *maskofX = (WidgetNotificationPanel*)maskof;
  
  set_angle(maskof->get_angle());
  set_origin(maskof->get_origin());
  
  glTranslatef(0,0,maskofX->anim_t);
  glRotatef(maskofX->anim_r,1,0,0);
  set_opacity_factor(maskofX->anim_a);
  RenderableGroup::render();
}

Renderable* WidgetNotificationPanel::get_mask() {
  return &mask;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, string *stream) {
  //cout << "recv: " << (char*)ptr << endl << endl;
  (*stream) += (char*)ptr;
  return size * nmemb;
}

WidgetNotificationPanel::WeatherUpdater::WeatherUpdater(WidgetNotificationPanel *wnp)
: Animator(60000*5),
  panel(wnp),
  weathericon(-1)
{
  Renderable::context->register_animator(this);
}

WidgetNotificationPanel::WeatherUpdater::~WeatherUpdater() {
  if(weathericon > -1)
    panel->general_information->detach(weatherimages[weathericon]);
  
  for(int i = 0; i < 4; ++i)
    delete weatherimages[i];
}

void WidgetNotificationPanel::WeatherUpdater::init() {
  temperature = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 20), 
    Vec2(235+50+80, 420-80-15));
  temperature->set_blendcol(RGBA(0,0,0,0.65));
  panel->general_information->attach(temperature);
  
  Rect r(235+50,420-80-35,235+50+64,420-80-35+64);
  weatherimages[0] = new Img("data/weather/cloudy.png", r);
  weatherimages[1] = new Img("data/weather/snowy.png", r);
  weatherimages[2] = new Img("data/weather/clear.png", r);
  weatherimages[3] = new Img("data/weather/rainy.png", r);
  
  for(int i = 0; i < 4; ++i)
    weatherimages[i]->set_blendcol(RGBA(1,1,1,0.65));
  
  loadingimage = new Img("data/weather/loading.png", r, 0.0f, false);
  loadingimage->set_blendcol(RGBA(0,0,0,0.65));
  panel->general_information->attach(loadingimage);
  temperature->get_stream().str("Updating");
}

int WidgetNotificationPanel::WeatherUpdater::weather_fetch_data(void *ptr) {
  SDL_Delay(10000); // sleep five seconds for giving the os time to connect to wifi after s2r resume
  WidgetNotificationPanel::WeatherUpdater *self = (WidgetNotificationPanel::WeatherUpdater*)ptr;
  
  CURL *curl;
  CURLcode res;
  string recv;
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.metaweather.com/api/location/551801/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  
  for(int i = recv.size() - 1; i >= 0 && recv[i] != '}'; --i)
    recv[i] = ' ';
  
  string err;
  auto json = Json::parse(recv, err);
  if(!err.empty()) {
    cout << "::Json::error: " << err << endl; 
    return 1;
  }
  
  float avg = 0.f;
  int iconcounter[4] = {0, 0, 0, 0};
  
  int target_icon = -1;
  for(auto x: json["consolidated_weather"].array_items()) {
    avg += x["the_temp"].number_value();
    
    string wstate = x["weather_state_abbr"].string_value();
    if(wstate == "hc" || wstate == "lc") { // heavy clouds, light clouds
      iconcounter[0]++;
      
      if(target_icon > -1) {
        if(iconcounter[3] > iconcounter[target_icon])
          target_icon = 0;
      }else
        target_icon = 0;
    }else if(wstate == "sn" || wstate == "h") { // snow hail
      iconcounter[1]++;
      
      if(target_icon > -1) {
        if(iconcounter[3] > iconcounter[target_icon])
          target_icon = 1;
      }else
        target_icon = 1;
    }else if(wstate == "c") { // clear
      iconcounter[2]++;
      
      if(target_icon > -1) {
        if(iconcounter[3] > iconcounter[target_icon])
          target_icon = 2;
      }else
        target_icon = 2;
    }else { // sleet, thunderstorm, heavy rain, light rain, showers
      iconcounter[3]++;
      
      if(target_icon > -1) {
        if(iconcounter[3] > iconcounter[target_icon])
          target_icon = 3;
      }else
        target_icon = 3;
    }
  }
  avg /= json["consolidated_weather"].array_items().size();
  
  self->temperature->get_stream().str(string());
  self->temperature->get_stream() << int(avg) << char(176) << "C";
  
  if(self->loadingimage) {
    self->panel->general_information->detach(self->loadingimage);
    self->loadingimage = NULL;
  }
  
  if(self->weathericon > -1)
    self->panel->general_information->detach(self->weatherimages[self->weathericon]);
  self->weathericon = target_icon;
  self->panel->general_information->attach(self->weatherimages[target_icon]);
  
  return 0;
}

void WidgetNotificationPanel::WeatherUpdater::step() {
  SDL_CreateThread(WidgetNotificationPanel::WeatherUpdater::weather_fetch_data, this);
}

WidgetNotificationPanel::WidgetNotificationPanel()
: mask(this),
  anim_r(0.0),
  anim_t(0.0),
  anim_a(1.0),
  Animator(10),
  toggle_state(true),
  weatherup(this),
  general_information(new RenderableGroup())
{
  date = new Text(Renderable::context->load_font("data/fonts/OpenSans-Light.ttf", 20), Vec2(235 + 50, 420 - 80 - 75));
  date->set_blendcol(RGBA(0,0,0,0.65));
  day = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 37), Vec2(235 + 50, 420 - 137 - 75));
  day->set_blendcol(RGBA(0,0,0,0.65));
  //RenderableGroup::attach(date);
  //RenderableGroup::attach(day);
  
  general_information->attach(date);
  general_information->attach(day);
  RenderableGroup::attach(general_information);
  
  Box *traybutton;
  traybutton = new Box(Rect(1024 - 24 - 32, 24, 1024 - 24, 24 + 32));
  traybutton->set_border_radius(8);
  traybutton->set_background_color(RGBA(0,0,0,0.6));
  traybutton->set_blendcol(RGBA(1,1,1,0.6));
  traybutton->append_texture(new ImageTexture("data/widgets/small/settings.png"));
  //RenderableGroup::attach(traybutton);
  
  traybutton = new Box(Rect(1024 - 24 - 32 - 38, 24, 1024 - 24 - 38, 24 + 32));
  traybutton->set_border_radius(8);
  traybutton->set_background_color(RGBA(0,0,0,0.6));
  traybutton->set_blendcol(RGBA(1,1,1,0.6));
  traybutton->append_texture(new ImageTexture("data/widgets/small/wifi.png"));
  //RenderableGroup::attach(traybutton);
  
  traybutton = new Box(Rect(1024 - 24 - 32 - 38*2, 24, 1024 - 24 - 38*2, 24 + 32));
  traybutton->set_border_radius(8);
  traybutton->set_background_color(RGBA(0,0,0,0.6));
  traybutton->set_blendcol(RGBA(1,1,1,0.6));
  traybutton->append_texture(new ImageTexture("data/widgets/small/speaker.png"));
  //RenderableGroup::attach(traybutton);
  
  //general_information->attach(new Img("data/weather/asd.png", Rect(300,300,364,364)));
    
  c = new MinimalisticClockWidget(Vec2(750, 270), 120); // big
  //c = new ClockWidget(Vec2(235 + 150 + 30, 70 + 150 + 30), 150); // small
  attach(c);
  
  weatherup.init();
}

void WidgetNotificationPanel::render() {
  static const char *s_dow[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"};

  static const char *s_mon[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"};
    
  date->get_stream().str(string());
  date->get_stream() << s_mon[c->get_month()] << "  " << c->get_day() << "  " << (c->get_year() + 1900);
  day->get_stream().str(s_dow[c->get_day_of_week()]);

  glTranslatef(0,0,anim_t);
  glRotatef(anim_r,1,0,0);
  set_opacity_factor(anim_a);
  Widget::render();
}

void WidgetNotificationPanel::hide_panel() {
  if(!toggle_state)
    return;
  
  c->pause();

  toggle_state = false;
  Renderable::context->register_animator(this);
}

void WidgetNotificationPanel::show_panel() {
  if(toggle_state)
    return;
  
  c->play();
  
  anim_r = 0;
  anim_t = 0;
  anim_a = 0;

  toggle_state = true;
  show();
  Renderable::context->register_animator(this);
}

