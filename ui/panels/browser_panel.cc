#include "browser_panel.h"
#include "../widgets/button.h"
#include "../box.h"
#include "../utils.h"
#include <iostream>
#include <SDL/SDL.h>
#include <string>
#include <list>
#include <gdk/gdkkeysyms.h>

using namespace std;

#define BROWSER_WIDTH (1024 - 72)
#define BROWSER_HEIGTH 600

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

SimpleHandler* g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler()
    : is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

bool SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
  rect = CefRect(0, 0, BROWSER_WIDTH, BROWSER_HEIGTH);
  return true;
}

MemoryTexture* SimpleHandler::get_main_browser_target() {
  static MemoryTexture *mainBrowserTarget = new MemoryTexture(100,100,32);
  return mainBrowserTarget;
}

void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) {
  /*if(!do_paint)
    return;
  */
  get_main_browser_target()->update_from_buffer((GLuint*)buffer, width, height);
  Renderable::context->refresh();
  //cout << "::CEF::browser paint " << get_main_browser_target() << endl;
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool SimpleHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString& target_url,
                                 const CefString& target_frame_name,
                                 WindowOpenDisposition target_disposition,
                                 bool user_gesture,
                                 const CefPopupFeatures& popupFeatures,
                                 CefWindowInfo& windowInfo,
                                 CefRefPtr<CefClient>& client,
                                 CefBrowserSettings& settings,
                                 bool* no_javascript_access)
{
  frame->LoadURL(target_url);
  return true;
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    //CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message.
  /*std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL " << std::string(failedUrl) <<
        " with error " << std::string(errorText) << " (" << errorCode <<
        ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);*/
  string googlequery("https://www.google.at/search?q=");
  googlequery += last_lookup;
  frame->LoadURL(googlequery);
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}

SimpleApp::SimpleApp(): initialized_(false), targetPanel(NULL) {
}

void SimpleApp::OnContextInitialized() {
  cout << "::CEF3::SimpleApp: context initialized" << endl;

  CEF_REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;
  window_info.SetAsWindowless(0, false);

  // SimpleHandler implements browser-level callbacks.
  handler = new SimpleHandler();

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  std::string url;

  // Check if a "--url=" value was provided via the command-line. If so, use
  // that instead of the default URL.
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  url = command_line->GetSwitchValue("url");
  if (url.empty())
    url = "http://www.google.com";

  // Create the first browser window.
  browser = CefBrowserHost::CreateBrowserSync(window_info, handler.get(), url,
                                          browser_settings, NULL);
  
  initialized_ = true;
  if(targetPanel) {
    cout << "attaching texture " << handler->get_main_browser_target() << endl;
    targetPanel->browserView->append_texture(handler->get_main_browser_target(), 
                                             TexCoords(Vec2(0,0),
                                                       Vec2(1,0),
                                                       Vec2(1,1),
                                                       Vec2(0,1)));
    //targetPanel->browserView->set_blendcol(RGBA(0,1,1,1));
  }
  
  browser->GetHost()->WasHidden(true);
  newpage = true;
}

void SimpleApp::OnFocusedNodeChanged( CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefDOMNode > node ) {
  if(!targetPanel)
    return;
  
  if(targetPanel->hidden())
    return;
  
  if(newpage) {
    newpage = false;
    browser->GetMainFrame()->ExecuteJavaScript("document.activeElement.blur();", browser->GetMainFrame()->GetURL(), 0);
    return;
  }
  if(node) {
    //cout << "note name: '" << node->GetElementTagName().ToString() << '"' << endl;
    if(node->GetElementTagName().ToString() == "INPUT" ||
       node->GetElementTagName().ToString() == "input")
    {
      //cout << "note type: '" << node->GetElementAttribute("TYPE").ToString() << '"' << endl;
      if(node->HasElementAttribute("TYPE")) {
        if(node->GetElementAttribute("TYPE") == "text" 
        || node->GetElementAttribute("TYPE") == "TEXT"
        || node->GetElementAttribute("TYPE") == "search" 
        || node->GetElementAttribute("TYPE") == "SEARCH") {
          targetPanel->keyboard->launch(node->GetValue().ToString());
        }
      }else {
        targetPanel->keyboard->launch(node->GetValue().ToString());
        //frame->ExecuteJavaScript("document.activeElement.value='test123';", frame->GetURL(), 0);
      }
      //cout << node->GetValue().ToString() << endl;
    }
  }
}

void SimpleApp::ConnectToPanel(BrowserPanel *p) {
  if(targetPanel)
    return;

  targetPanel = p;
  if(initialized_) {
    cout << "attaching texture " << handler->get_main_browser_target() << endl;
    targetPanel->browserView->append_texture(handler->get_main_browser_target(), 
                                             TexCoords(Vec2(0,0),
                                                       Vec2(1,0),
                                                       Vec2(1,1),
                                                       Vec2(0,1)));
  }
}

class LookupButton: public Button {
    BrowserPanel *bp;
  public:
    LookupButton(Img *i, BrowserPanel *_bp)
    : Button(72, 72, i),
      bp(_bp)
    {
      translate(Vec2(0,72));
      url_keyboard = new UrlKeyboard(bp);
    }
    
    void callback() {
      if(!bp->app)
        return;
      url_keyboard->launch();
    }
    
    class UrlKeyboard: public Keyboard {
      BrowserPanel *bp;
    public:
      UrlKeyboard(BrowserPanel *_bp): bp(_bp) {}
      void on_confirm(std::string s) {
        bp->app->browser->GetMainFrame()->LoadURL(s);
        bp->app->SetNewPageFlag();
        bp->app->handler->SetLastLookup(s);
      }
      void on_panel_hide() {
        bp->app->browser->GetHost()->WasHidden(false);
      }
      void on_panel_show() {
        bp->app->browser->GetHost()->WasHidden(true);
      }
    };
    
    UrlKeyboard *url_keyboard;
};

class BackButton: public Button {
    Img *prepare_img() {
      Img *img = new Img("data/widgets/browserback.png", Rect(0,0,32,32));
      img->set_blendcol(RGBA(1,1,1,0.8));
      return img;
    }
    BrowserPanel *panel;
  public:
    BackButton(BrowserPanel *_panel): Button(72, 72, prepare_img()), panel(_panel) {
      translate(Vec2(0,72*2));
    }
    
    void callback() {
      panel->app->browser->GoBack();
    }
};

class BookmarksButton: public Button {
    Img *prepare_img() {
      Img *img = new Img("data/widgets/favorite.png", Rect(0,0,32,32));
      img->set_blendcol(RGBA(1,1,1,0.8));
      return img;
    }
    BrowserPanel *panel;
  public:
    BookmarksButton(BrowserPanel *_panel): Button(72, 72, prepare_img()), panel(_panel) {
      translate(Vec2(0,72*3));
    }
    
    void callback() {
    }
};

BrowserPanel::BrowserPanel(SimpleApp *_app)
: AppPanel(RGBA("#2ecc71", 1), RGBA("#27ae60", 1)),
  oldev(0,0,0),
  draging(false),
  app(_app),
  keyboard(new SimpleKeyboard(this))
{ 
  browserView = new Box(Rect(72, 0, 1024, 600));
  RenderableGroup::attach(browserView);

  Img *lookup = new Img("data/widgets/lookup.png", Rect(0,0,32,32));
  lookup->set_blendcol(RGBA(1,1,1,0.8));
  LookupButton *lookup_btn = new LookupButton(lookup, this);
  
  attach(lookup_btn);
  
  attach(new BackButton(this));
  attach(new BookmarksButton(this));
  
  attach(lookup_btn->url_keyboard);
  attach(keyboard);
  
  
  if(app) {
    app->ConnectToPanel(this);
  }
}

void BrowserPanel::on_panel_show() { 
  app->browser->GetHost()->WasHidden(false);
}

void BrowserPanel::on_panel_hide() {
  app->browser->GetHost()->WasHidden(true);
}

void BrowserPanel::on_mouse_up(const MouseEvent &ev) {
  if(!app)
    return;
    
  if(draging) {
    draging = false;
    return;
  }
    
  CefMouseEvent mev;
  mev.x = ev.get_x() - browserView->get_shape().left;
  mev.y = ev.get_y() - browserView->get_shape().top;
  mev.modifiers = 0;
  app->browser->GetHost()->SendMouseClickEvent(
    mev, 
    CefBrowserHost::MouseButtonType::MBT_LEFT,
    true,
    1);
}
  
void BrowserPanel::on_mouse_drag_motion(const MouseEvent &ev) {
  if(!app)
    return;
  
  CefMouseEvent mev;
  mev.x = ev.get_x() - browserView->get_shape().left;
  mev.y = ev.get_y() - browserView->get_shape().top;
  mev.modifiers = 0;
  
  float deltax = ev.get_x() - oldev.get_x(),
        deltay = ev.get_y() - oldev.get_y();
  //browser->GetHost().get()->SendMouseMoveEvent(mev, true);
  app->browser->GetHost()->SendMouseWheelEvent(mev, deltax, deltay);
  oldev = ev;
  draging = true;
}
  
void BrowserPanel::on_mouse_down(const MouseEvent &ev) {
  if(!app)
    return;
  
  CefMouseEvent mev;
  mev.x = ev.get_x() - browserView->get_shape().left;
  mev.y = ev.get_y() - browserView->get_shape().top;
  mev.modifiers = 0;
  //cout << mev.x << ", " << mev.y << endl;
  app->browser->GetHost()->SendMouseClickEvent(
    mev, 
    CefBrowserHost::MouseButtonType::MBT_LEFT,
    false,
    1);
  oldev = ev;
  
  
  //cout << "send key event" << endl;
      /*  CefKeyEvent key;
      key.type = KEYEVENT_CHAR;
      key.modifiers =0;
      key.native_key_code = 'A';
      app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);
      app->browser->GetHost()->SendKeyEvent(key);*/
}

void BrowserPanel::SimpleKeyboard::on_keypress(int keycode) {
  /*if(keycode < 128) {
    CefKeyEvent key;
    key.type = KEYEVENT_CHAR;
    key.modifiers =0;
    key.character = keycode;
    key.native_key_code = GDK_KEY_Down;
    bp->app->browser->GetHost()->SendKeyEvent(key);
  }else if(keycode == SpecialKey::key_type_t::SpaceBar) {
    CefKeyEvent key;
    key.type = KEYEVENT_CHAR;
    key.modifiers =0;
    key.character = ' ';
    key.native_key_code = GDK_KEY_Down;
    bp->app->browser->GetHost()->SendKeyEvent(key);
  }*/
}

void BrowserPanel::SimpleKeyboard::on_confirm(string s) {
  string script("document.activeElement.value='");
  script += s + "';";
  bp->app->browser->GetMainFrame()->ExecuteJavaScript(script, bp->app->browser->GetMainFrame()->GetURL(), 0);
}

BrowserPanel::~BrowserPanel() {
  
}

