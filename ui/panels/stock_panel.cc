#include "stock_panel.h"
#include "../widgets/button.h"
#include "../widgets/list.h"
#include "../geom.h"
#include "../../storage/storage.h"
#include "../../network/mailer.h"

#include <iostream>

#define PRODUCT_PANEL_BUTTON_SIZE 99

using namespace std;

class ShoppingListItem: public ListItemText {
  int id;
public:
  ShoppingListItem(int _id, int _count, string s)
  : ListItemText(s),
    id(_id)
  {}
  
  void callback() {}
  
  void on_removed() {
    Storage::instance().cmd() << "DELETE FROM shopping_list WHERE id=" << id << ";";
    Storage::instance().exec();
  }
  
  void add_counter(int count) {
    Text *countcap = new Text(Renderable::context->load_font("data/fonts/OpenSans-Regular.ttf", 13), 
      Vec2(12 + caption->get_width() + 12, (height - 13)*0.5));
    countcap->set_blendcol(RGBA(.6f,.6f,.6f,0.7));
    countcap->get_stream() << "x" << count;
    if(get_container())
      get_container()->attach(countcap);
  }
};

class ListStockItem: public ListItemText {
  StockPanel *panel;
  int id;
public:
  ListStockItem(StockPanel *_panel, int _id, string s)
  : ListItemText(s),
    panel(_panel),
    id(_id)
  {}
  
  void callback() {
    panel->add_to_shopping_list(id);
  }
  
  void on_removed() {
    Storage::instance().cmd() << "DELETE FROM stock_items WHERE id=" << id << ";";
    Storage::instance().exec();
  }
};

void StockKeyboard::on_confirm(string s) {
  Storage::CmdResult res;
  Storage::instance().cmd() << "SELECT id FROM stock_categories WHERE name='" << category << "';";
  Storage::instance().exec(&res);
  
  if(res["id"].size() > 0) {
    Storage::instance().cmd() <<
      "INSERT INTO stock_items(id_category, name)" <<
      " SELECT " << res["id"][0] << ", '" << s << "' WHERE NOT EXISTS(SELECT 1 FROM stock_items WHERE name='" << s << "' AND id_category=" << res["id"][0] << ");";
    Storage::instance().exec();
  }
}

class ShoppingButton: public Button {
  public:
    ShoppingButton(Img *i): Button(72, 72, i) {
      translate(Vec2(0,72));
    }
    
    void callback() {
      cout << "manage shopping list" << endl;
    }
};

class ReadInventoryButton: public Button {
  public:
    ReadInventoryButton(Img *i): Button(72, 72, i) {
      translate(Vec2(0,72 * 2));
    }
    
    void callback() {
      cout << "read inventory items" << endl;
    }
};

class SendAsMailButton: public Button {
    StockPanel *panel;
  public:
    SendAsMailButton(Img *i, StockPanel *_panel): Button(72, 72, i) {
      translate(Vec2(0,72 * 3));
      panel = _panel;
    }
    
    void callback() {
      std::string from, to, data;
      
      Storage::instance().get_setting("grocery_list_email_to", to);
      Storage::instance().get_setting("grocery_list_email_from", from);
      
      Storage::CmdResult res;
      Storage::instance().cmd() << "SELECT si.name,sl.count FROM shopping_list AS sl INNER JOIN stock_items AS si WHERE si.id=sl.id_item ORDER BY si.id_category,si.name;";
      Storage::instance().exec(&res);
      
      for(int i = 0; i < res["name"].size(); ++i)
        data += res["name"][i] + ", x" + res["count"][i] + "\r\n";
      
      cout << data << endl;
      Mailer(to, from, "Grocery list", data);
      
      Storage::instance().cmd() << "DELETE FROM shopping_list;";
      Storage::instance().exec();
      panel->add_to_shopping_list(-1);
      Renderable::context->refresh();
    }
};

class ProductCategoryButton: public Button {
  StockPanel *panel;
  Circle *spinner;
  string category_name;
  
  Img* get_image(std::string s) {
    Rect r(0,0,42,42);
    if(s == "Vegetables")
      return new Img("data/food/vegetables.png", r);
    else if(s == "Fruits")
      return new Img("data/food/fruits.png", r);
    else if(s == "Mushrooms")
      return new Img("data/food/mushrooms.png", r);
    else if(s == "Pastries")
      return new Img("data/food/pastries.png", r);
    else if(s == "Wheatproducts")
      return new Img("data/food/wheatproducts.png", r);
    else if(s == "Oil")
      return new Img("data/food/oil.png", r);
    else if(s == "Sweets")
      return new Img("data/food/sweet.png", r);
    else if(s == "Spices")
      return new Img("data/food/spices.png", r);
    else if(s == "Snacks")
      return new Img("data/food/snacks.png", r);
    else if(s == "Eggs")
      return new Img("data/food/eggs.png", r);
    else if(s == "Meat")
      return new Img("data/food/meat.png", r);
    else if(s == "Dairy products")
      return new Img("data/food/milkproducts.png", r);
    else if(s == "Fish")
      return new Img("data/food/fish.png", r);
    else if(s == "Honey")
      return new Img("data/food/honey.png", r);
    else if(s == "Non-alcoholic beverages")
      return new Img("data/food/nonalcoholic.png", r);
    else if(s == "Alcoholic beverages")
      return new Img("data/food/alcoholicbev.png", r);
    else if(s == "Salt")
      return new Img("data/food/salt.png", r);
    else if(s == "Canned food")
      return new Img("data/food/instantfood.png", r);
    return NULL;
  }
  
  class OpenSubMenuSpinnerAnimator: public Animator {
    float hl;
    ProductCategoryButton *btn;
    bool till_end;
  public:
    OpenSubMenuSpinnerAnimator(ProductCategoryButton *_btn):
    Animator(10), hl(0.0f), btn(_btn), till_end(false) {}
    
    void step() {
      btn->spinner->show();
      hl += 0.005;
      if(hl >= 0.4f) {
        hl = 0.4f;
        till_end = true;
        unregister();
        btn->spinner->hide();
        btn->panel->get_keyboard()->launch(btn->category_name);
        btn->panel->get_list()->resize(1024 - (PRODUCT_PANEL_BUTTON_SIZE + 1) * 3 - 72);
        btn->panel->get_choices_list()->hide();
      }
      
      btn->spinner->section_length(2 * M_PI * hl / 0.4f);
      btn->spinner->set_border_color(RGBA(1,1,1, hl));
    }
    
    void reset() {
      hl = 0.0f;
      till_end = false;
    }
    
    bool reached_end() {
      return till_end;
    }
  };
  OpenSubMenuSpinnerAnimator spinner_anim;
  
  friend class OpenSubMenuSpinnerAnimator;
public:
  ProductCategoryButton(int row, int col, StockPanel *_panel, string s)
  : Button(PRODUCT_PANEL_BUTTON_SIZE,PRODUCT_PANEL_BUTTON_SIZE,get_image(s)),
    panel(_panel),
    //spinner(new Circle(Vec2(PRODUCT_PANEL_BUTTON_SIZE,PRODUCT_PANEL_BUTTON_SIZE)*0.5f,PRODUCT_PANEL_BUTTON_SIZE*0.5f-15,100)),
    spinner_anim(this),
    category_name(s)
  {
    Vec2 pos(1025 - (PRODUCT_PANEL_BUTTON_SIZE + 1) * (col + 1),
             (PRODUCT_PANEL_BUTTON_SIZE + 1) * row);
    translate(pos);
    spinner = new Circle(Vec2(PRODUCT_PANEL_BUTTON_SIZE,PRODUCT_PANEL_BUTTON_SIZE)*0.5f,PRODUCT_PANEL_BUTTON_SIZE*0.5f-15,50);
    spinner->set_blendcol(RGBA(0,0,0,0));
    spinner->set_border_width(10);
    RenderableGroup::attach(spinner);
    spinner->hide();
    
    Storage::instance().cmd() <<
      "INSERT INTO stock_categories(name)" <<
      " SELECT '" << s << "' WHERE NOT EXISTS(SELECT 1 FROM stock_categories WHERE name='" << s << "');";
    Storage::instance().exec();
  }
  
  void on_mouse_down(const MouseEvent &ev) {
    Button::on_mouse_down(ev);
    spinner_anim.reset();
    Renderable::context->register_animator(&spinner_anim);
  }
  
  void on_mouse_up(const MouseEvent &ev) {
    Button::on_mouse_up(ev);
    spinner_anim.unregister();
    spinner->hide();
    Renderable::context->refresh();
  }

  void callback() { 
    if(spinner_anim.reached_end())
      return;
    
    Storage::CmdResult res;
    Storage::instance().cmd() << "SELECT i.id,i.name FROM stock_categories AS c INNER JOIN stock_items AS i ON i.id_category=c.id WHERE c.name='" << category_name << "' ORDER BY i.name;";
    Storage::instance().exec(&res);
    
    if(res["id"].size() > 1) {
      panel->get_choices_list()->clear();
      for(int i = 0; i < res["id"].size(); ++i) {
        int id = atoi(res["id"][i].c_str());
        panel->get_choices_list()->append_item(new ListStockItem(panel, id, res["name"][i]));
      }
      
      panel->get_list()->resize(320);
      panel->get_choices_list()->show();
    }else if(res["id"].size() == 1) {
      panel->get_list()->resize(1024 - (PRODUCT_PANEL_BUTTON_SIZE + 1) * 3 - 72);
      panel->get_choices_list()->hide();
      
      panel->add_to_shopping_list(atoi(res["id"][0].c_str()));
    }else {
      panel->get_list()->resize(1024 - (PRODUCT_PANEL_BUTTON_SIZE + 1) * 3 - 72);
      panel->get_choices_list()->hide();
    }
    //panel->get_choices_list()->append_item(new ListItemText(category_name, 45));
  }
};

StockPanel::StockPanel(): AppPanel(RGBA("#e67e22", 1), RGBA("#d35400", 1)) {
  Storage::instance().exec("CREATE TABLE IF NOT EXISTS stock_categories (id INTEGER PRIMARY KEY, name TEXT);");
  Storage::instance().exec("CREATE TABLE IF NOT EXISTS stock_items (id INTEGER PRIMARY KEY, id_category INT, name TEXT, FOREIGN KEY(id_category) REFERENCES stock_categories(id));");
  Storage::instance().exec("CREATE TABLE IF NOT EXISTS shopping_list (id INTEGER PRIMARY KEY, id_item INT, count INT DEFAULT 0, FOREIGN KEY(id_item) REFERENCES stock_items(id));");

  attach(new ProductCategoryButton(0, 0, this, "Vegetables"));
  attach(new ProductCategoryButton(1, 0, this, "Fruits"));
  attach(new ProductCategoryButton(2, 0, this, "Mushrooms"));
  attach(new ProductCategoryButton(3, 0, this, "Pastries"));
  attach(new ProductCategoryButton(4, 0, this, "Wheatproducts"));
  attach(new ProductCategoryButton(5, 0, this, "Oil"));
  attach(new ProductCategoryButton(0, 1, this, "Sweets"));
  attach(new ProductCategoryButton(1, 1, this, "Spices"));
  attach(new ProductCategoryButton(2, 1, this, "Snacks"));
  attach(new ProductCategoryButton(3, 1, this, "Eggs"));
  attach(new ProductCategoryButton(4, 1, this, "Meat"));
  attach(new ProductCategoryButton(5, 1, this, "Dairy products"));
  attach(new ProductCategoryButton(0, 2, this, "Fish"));
  attach(new ProductCategoryButton(1, 2, this, "Honey"));
  attach(new ProductCategoryButton(2, 2, this, "Non-alcoholic beverages"));
  attach(new ProductCategoryButton(3, 2, this, "Alcoholic beverages"));
  attach(new ProductCategoryButton(4, 2, this, "Salt"));
  attach(new ProductCategoryButton(5, 2, this, "Canned food"));
  
  Img *shoppinglist = new Img("data/widgets/shoppinglist.png", Rect(0,0,32,32));
  shoppinglist->set_blendcol(RGBA(1,1,1,0.8));
  Button *shoppinglist_btn = new ShoppingButton(shoppinglist);
  
  Img *readinventory = new Img("data/widgets/barcode.png", Rect(0,0,32,32));
  readinventory->set_blendcol(RGBA(1,1,1,0.8));
  Button *readinventory_btn = new ReadInventoryButton(readinventory);
  
  Img *sendmail = new Img("data/widgets/mail.png", Rect(0,0,32,32));
  sendmail->set_blendcol(RGBA(1,1,1,0.8));
  Button *sendasmail_btn = new SendAsMailButton(sendmail, this);
  
  attach(readinventory_btn);
  attach(shoppinglist_btn);
  attach(sendasmail_btn);
  
  lst = new List(Rect(72,0,1024 - (PRODUCT_PANEL_BUTTON_SIZE + 1) * 3,600));
  attach(lst);
  
  choices_lst = new List(Rect(397,0,1024 - (PRODUCT_PANEL_BUTTON_SIZE + 1) * 3,600));
  choices_lst->hide();
  attach(choices_lst);
  
  kbd = new StockKeyboard();
  attach(kbd);
  
  Storage::CmdResult res;
  Storage::instance().cmd() << "SELECT sl.id,si.name,sl.count FROM shopping_list AS sl INNER JOIN stock_items AS si ON sl.id_item=si.id ORDER BY si.id_category,si.name;";
  Storage::instance().exec(&res);
    
  lst->clear();
  for(int i = 0; i < res["id"].size(); ++i) {
    int id = atoi(res["id"][i].c_str()),
        count = atoi(res["count"][i].c_str());
    ShoppingListItem *sli = new ShoppingListItem(id, count, res["name"][i]);
    lst->append_item(sli);
    sli->add_counter(count);
    //lst->append_item(new ListItemText(res["name"][i]));
  }
}

void StockPanel::add_to_shopping_list(int id) {
  if(id >= 0) {
    Storage::instance().cmd() <<
      "INSERT INTO shopping_list(id_item)" <<
      " SELECT " << id << " WHERE NOT EXISTS(SELECT 1 FROM shopping_list WHERE id_item=" << id << ");";
    Storage::instance().exec();
    
    Storage::instance().cmd() <<
      "UPDATE shopping_list SET count=count+1 WHERE id_item=" << id << ";";
    Storage::instance().exec();
  }
 
  Storage::CmdResult res;
  Storage::instance().cmd() << "SELECT sl.id,si.name,sl.count FROM shopping_list AS sl INNER JOIN stock_items AS si ON sl.id_item=si.id ORDER BY si.id_category,si.name;";
  Storage::instance().exec(&res);
    
  lst->clear();
  for(int i = 0; i < res["id"].size(); ++i) {
    int id = atoi(res["id"][i].c_str()),
        count = atoi(res["count"][i].c_str());
    ShoppingListItem *sli = new ShoppingListItem(id, count, res["name"][i]);
    lst->append_item(sli);
    sli->add_counter(count);
  }
}

StockKeyboard* StockPanel::get_keyboard() {
  return kbd;
}

