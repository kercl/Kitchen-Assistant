#ifndef BROWSER_PANEL_H
#define BROWSER_PANEL_H

#include "../widgets/panel.h"
#include "../widgets/keyboard.h"
#include "../texture.h"
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

#include <SDL/SDL_thread.h>
#include <list>

class LookupButton;
class BackButton;
class BookmarksButton;
class BrowserPanel;

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRenderHandler {
  std::string last_lookup;
public:
  SimpleHandler();
  ~SimpleHandler();

  static SimpleHandler* GetInstance();

  bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect);
  void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height);

  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE {
    return this;
  }
  
  virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             const CefString& target_url,
                             const CefString& target_frame_name,
                             WindowOpenDisposition target_disposition,
                             bool user_gesture,
                             const CefPopupFeatures& popupFeatures,
                             CefWindowInfo& windowInfo,
                             CefRefPtr<CefClient>& client,
                             CefBrowserSettings& settings,
                             bool* no_javascript_access) OVERRIDE;

  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;

  void CloseAllBrowsers(bool force_close);
  void SetLastLookup(std::string s) { last_lookup = s; }
  bool IsClosing() const { return is_closing_; }

  MemoryTexture* get_main_browser_target();
 private:
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList browser_list_;

  bool is_closing_;
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler {
  bool newpage;
public:
  SimpleApp();
  
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE { return this; }
  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
      OVERRIDE { return this; }

  virtual void OnContextInitialized() OVERRIDE;
  virtual void OnFocusedNodeChanged( CefRefPtr< CefBrowser > browser, CefRefPtr< CefFrame > frame, CefRefPtr< CefDOMNode > node );
  
  void ConnectToPanel(BrowserPanel *p);

  CefRefPtr<SimpleHandler> handler;
  CefRefPtr<CefBrowser> browser;
  
  void SetNewPageFlag() { newpage = true; }
 private:
  IMPLEMENT_REFCOUNTING(SimpleApp);
  BrowserPanel *targetPanel;
  bool initialized_;
};


class BrowserPanel: public AppPanel {
  SimpleApp *app;
  Box *browserView;
  
  MouseEvent oldev;
  bool draging;
  
  class SimpleKeyboard: public Keyboard {
    BrowserPanel *bp;
  public:
    SimpleKeyboard(BrowserPanel *_bp): bp(_bp) {}
    void on_panel_hide() {
      bp->app->browser->GetHost()->WasHidden(false);
    }
    void on_panel_show() {
      bp->app->browser->GetHost()->WasHidden(true);
    }
    void on_keypress(int keycode);
    void on_confirm(std::string s);
  };
  
  SimpleKeyboard *keyboard;
  
  friend class LookupButton;
  friend class BackButton;
  friend class BookmarksButton;
  friend class SimpleKeyboard;
  friend class SimpleApp;
public:
  BrowserPanel(SimpleApp *_app);
  ~BrowserPanel();
  
  void on_panel_show();
  void on_panel_hide();
  
  virtual void on_mouse_down(const MouseEvent &ev);
  virtual void on_mouse_up(const MouseEvent &ev);
  virtual void on_mouse_drag_motion(const MouseEvent &ev);
};

#endif

