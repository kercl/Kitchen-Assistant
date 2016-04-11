#ifndef STORAGE_H
#define STORAGE_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include <map>
#include <deque>

class Storage {
public:
  typedef std::map<std::string, std::deque<std::string>> CmdResult;
private:
  sqlite3 *db;
  std::stringstream exec_cmd;
  
  Storage();
  
  static int callback(void *notUsed, int argc, char **argv, char **azColName);
public:
  ~Storage();
  
  static Storage& instance();
  
  void exec(std::string command, CmdResult *target = NULL);
  void exec(CmdResult *target = NULL);
  std::stringstream& cmd();
  
  void get_setting(std::string key, std::string &target, std::string defaultval = "");
  void get_setting(std::string key, int &target, int defaultval = 0);
  void get_setting(std::string key, float &target, float defaultval = 0.0f);
  
  void set_setting(std::string key, std::string val);
  void set_setting(std::string key, int val);
  void set_setting(std::string key, float val);
  
  void cleanup();
};

class PersistentSettings {
public:
  ~PersistentSettings();
  virtual void snapshot() = 0;
};

#endif

