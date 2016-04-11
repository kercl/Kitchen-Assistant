#include "settings_panel.h"
#include "../widgets/button.h"
#include <iostream>

#define PRODUCT_PANEL_BUTTON_SIZE 99

using namespace std;

class RestartButton: public Button {
    Img *prepare_img() {
      Img *img = new Img("data/widgets/restart.png", Rect(0,0,32,32));
      img->set_blendcol(RGBA(1,1,1,0.8));
      return img;
    }
  public:
    RestartButton(): Button(72, 72, prepare_img()) {
      translate(Vec2(0,72));
    }
    
    void callback() {
      Renderable::context->mark_for_termination();
    }
};

SettingsPanel::SettingsPanel(): AppPanel(RGBA("#9b59b6", 1), RGBA("#8e44ad", 1)) {
  attach(new RestartButton());
}

