#ifndef STOCK_PANEL_H
#define STOCK_PANEL_H

#include "../widgets/panel.h"
#include "../widgets/keyboard.h"
#include "../widgets/list.h"

#include <string>

class StockKeyboard: public Keyboard {
  std::string category;
public:
  virtual void on_confirm(std::string s);
  void launch(std::string s) { category = s; Keyboard::launch(); }
};

class StockPanel: public AppPanel {
  StockKeyboard *kbd;
  List *lst, *choices_lst;
public:
  StockPanel();
  StockKeyboard* get_keyboard();
  List* get_list() { return lst; }
  List* get_choices_list() { return choices_lst; }
  
  void add_to_shopping_list(int id);
};

#endif

