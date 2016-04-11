#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <cmath>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <curl/curl.h>

#include "ui/render.h"
#include "ui/img.h"
#include "ui/utils.h"
#include "ui/box.h"
#include "ui/shader.h"
#include "ui/widgets/navigation.h"
#include "ui/widgets/notification.h"
#include "ui/widgets/clock.h"
#include "ui/panels/alarm_panel.h"
#include "ui/panels/entertainment_panel.h"
#include "ui/panels/stock_panel.h"
#include "ui/panels/browser_panel.h"
#include "ui/panels/settings_panel.h"
#include "storage/storage.h"

using namespace std;

RenderContext mr;
bool locked = false;

void idle() {
  if(locked)
    return;
  locked = true;
  int i = 0;
  for(auto x: mr.get_animators()) {
    x->invoke();
    i++;
  }
  locked = false;
  
  CefDoMessageLoopWork();
}

void on_mouse_event(int button, int state, int x, int y) {
  if(state == SDL_MOUSEBUTTONDOWN) {
    mr.cast_event(MouseDownEvent(button, x, y));
  }else if(state == SDL_MOUSEBUTTONUP) {
    mr.cast_event(MouseUpEvent(button, x, y));
  }
}

void on_mouse_drag_motion(int x, int y) {
  mr.cast_event(MouseMoveEvent(x, y));
}

void sig_handler(int signo) {
  if(signo == SIGQUIT)
    mr.mark_for_termination();
}

int main(int argc, char **argv) {
  srand(time(NULL));
  curl_global_init(CURL_GLOBAL_ALL);
  
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    cout << "Oops, massive problems encountered!" << endl;
    return 0;
  }
  
  bool show_cursor = true;
  int flags = SDL_OPENGL;
  if(argc > 1) {
    for(int i = 1; i < argc; ++i) {
      if(!strcmp(argv[i], "fullscreen"))
        flags |= SDL_FULLSCREEN;
      else if(!strcmp(argv[i], "nocursor"))
        show_cursor = false;
    }
  }

  CefMainArgs main_args(argc, argv);
  int exit_code = CefExecuteProcess(main_args, NULL, NULL);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.single_process = true;
  CefString(&settings.user_agent) = "Mozilla/5.0 (Linux; U; Android 4.0.3; ko-kr; LG-L160L Build/IML74K) KitchenPC/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30";

  CefRefPtr<SimpleApp> app(new SimpleApp);
  CefInitialize(main_args, settings, app.get(), NULL);

  SDL_SetVideoMode(1024, 600, 32, flags);
  
  int32_t cursorData[2] = {0, 0};
  SDL_Cursor *cursor = SDL_CreateCursor((Uint8 *)cursorData, (Uint8 *)cursorData, 8, 8, 4, 4);
  if(show_cursor == false)
    SDL_SetCursor(cursor);

  glClearColor(0,0,0,0);
  glLoadIdentity();
  
  glEnable (GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  Renderable *main_pass[4];
  
  EntertainmentPanel *entertainment_panel = new EntertainmentPanel();
  AlarmPanel *alarm_panel = new AlarmPanel(entertainment_panel);
  StockPanel *stock_panel = new StockPanel();
  BrowserPanel *browser_panel = new BrowserPanel(app.get());
  SettingsPanel *settings_panel = new SettingsPanel();
  
  vector<WidgetNavigationButton*> navigation_btns = {
    new WidgetNavigationButtonAlarm(alarm_panel),
    new WidgetNavigationButtonEntertainment(entertainment_panel),
    new WidgetNavigationButtonShoppingList(stock_panel),
    new WidgetNavigationButtonBrowser(browser_panel),
    new WidgetNavigationButtonSettings(settings_panel)};
  
  main_pass[0] = new Wallpaper({"data/wallpapers/moss.jpg"});
  main_pass[1] = new WidgetNotificationPanel();
  main_pass[2] = new WidgetNavigation((WidgetNotificationPanel*)main_pass[1], 
                                      (Wallpaper*)main_pass[0],
                                      navigation_btns);
  main_pass[3] = NULL;
  
  for(int i = 0; i < sizeof(main_pass) / sizeof(Renderable*); ++i)
    mr.push(main_pass[i]);
  
  mr.push(alarm_panel);
  mr.push(entertainment_panel);
  mr.push(stock_panel);
  mr.push(browser_panel);
  mr.push(settings_panel);
	  
	bool loop = true;
	
	signal(SIGQUIT, sig_handler);
	
	SDL_Event windowEvent;
  while(loop) {
    while(SDL_PollEvent(&windowEvent)) {
      if(windowEvent.type == SDL_QUIT)
        loop = false;
      else if(windowEvent.type == SDL_MOUSEMOTION)
        on_mouse_drag_motion(windowEvent.motion.x, windowEvent.motion.y);
      else if(windowEvent.type == SDL_MOUSEBUTTONDOWN || 
              windowEvent.type == SDL_MOUSEBUTTONUP)
        on_mouse_event(windowEvent.button.button, 
                       windowEvent.button.type, 
                       windowEvent.button.x,
                       windowEvent.button.y);
    }
    if(mr.terminate())
      loop = false;
    
    idle();
    mr.render();
    
    SDL_Delay(1);
  }
  CefShutdown();
  //SDL_Quit(); // segfaults, not thread safe probably, TODO
  
	return 0;
}

