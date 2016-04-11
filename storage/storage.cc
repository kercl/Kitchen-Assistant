#include "storage.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

Storage& Storage::instance() {
  static Storage s;
  return s;
}

int Storage::callback(void *notUsed, int argc, char **argv, char **azColName) {
  //int i;
  
  //for(i=0; i<argc; i++){
    //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    //Storage::instance().last_result[azColName[i]].push_back(argv[i] ? argv[i] : "NULL");
  //}
  //printf("\n");
  //cout << Storage::base << endl;
  /*
  for(auto x: Storage::instance().last_result.at("seconds"))
      cout << x << endl;*/
  return 0;
}

Storage::Storage() {
  cout << "::Storage::initializing database" << endl;
  
  if(sqlite3_open("data/kiosk.db", &db)){
    cerr << "Opening database failed" << endl;
    sqlite3_close(db);
    db = NULL;
    return;
  }
  
  exec("CREATE TABLE IF NOT EXISTS settings (key TEXT UNIQUE, str_val TEXT DEFAULT '', int_val INT DEFAULT 0, float_val FLOAT DEFAULT 0);");
}

Storage::~Storage() {
  cout << "::Storage::Closeing database" << endl;
  sqlite3_close(db);
}

void Storage::cleanup() {
  /*if(Storage::base != NULL) {
    sqlite3_close(db);
    db = NULL;
  }*/
}
  
void Storage::exec(std::string command, CmdResult *target) {
  if(db == NULL || command.length() == 0)
    return;
  
  cout << "::Storage::exec(" << target << "): " << command << endl;

  /*if(!target) {
    char *zErrMsg = 0;
    //int rc = sqlite3_exec(db, command.c_str(), callback, 0, &zErrMsg);
    
    if(rc != SQLITE_OK){
      cerr << "::Storage::error: " << zErrMsg << endl;
      sqlite3_free(zErrMsg);
    }
  }else {*/
  sqlite3_stmt *statement;
   
  if(sqlite3_prepare_v2(db, command.c_str(), command.length(), &statement, 0) == SQLITE_OK) {
    int cols = sqlite3_column_count(statement);

   if(target) {
      while(sqlite3_step(statement) == SQLITE_ROW) {
        for(int i = 0; i < cols; ++i) {
          (*target)[sqlite3_column_name(statement, i)].push_back(string((char*)sqlite3_column_text(statement, i)));
          //cout << sqlite3_column_name(statement, i) << " --> " << sqlite3_column_text(statement, i) << endl;
        }
      }
    }else if(sqlite3_step(statement) != SQLITE_ROW) {
      cout << "::Storage::error" << endl;
    }
      
    sqlite3_finalize(statement);
  }else {
    cout << "::Storage::error" << endl;
  }
  //}
}

void Storage::exec(CmdResult *target) {
  cout << "Target: " << target << endl;
  exec(exec_cmd.str(), target);
}

std::stringstream& Storage::cmd() {
  exec_cmd.str(string());
  return exec_cmd;
}

void Storage::get_setting(std::string key, std::string &target, std::string defaultval) {
  stringstream ss;
  ss << "SELECT str_val FROM settings WHERE key=='"
     << key << "'";
     
  sqlite3_stmt *statement;
  target = defaultval;
  
  if(sqlite3_prepare_v2(db, ss.str().c_str(), -1, &statement, 0) == SQLITE_OK) {
    int cols = sqlite3_column_count(statement);

    if(cols == 1) {
      int result = sqlite3_step(statement);
      if(result == SQLITE_ROW)
        target = string((char*)sqlite3_column_text(statement, 0));
    }
  }
}

void Storage::get_setting(std::string key, int &target, int defaultval) {
  stringstream ss;
  ss << "SELECT int_val FROM settings WHERE key=='"
     << key << "'";
     
  sqlite3_stmt *statement;
  target = defaultval;
  
  if(sqlite3_prepare_v2(db, ss.str().c_str(), -1, &statement, 0) == SQLITE_OK) {
    int cols = sqlite3_column_count(statement);

    if(cols == 1) {
      int result = sqlite3_step(statement);
      if(result == SQLITE_ROW) {
        istringstream ss((char*)sqlite3_column_text(statement, 0));
        ss >> target;
      }
    }
  }
}

void Storage::get_setting(std::string key, float &target, float defaultval) {
  stringstream ss;
  ss << "SELECT float_val FROM settings WHERE key=='"
     << key << "'";
     
  sqlite3_stmt *statement;
  target = defaultval;
  
  if(sqlite3_prepare_v2(db, ss.str().c_str(), -1, &statement, 0) == SQLITE_OK) {
    int cols = sqlite3_column_count(statement);
    if(cols == 1) {
      int result = sqlite3_step(statement);
      if(result == SQLITE_ROW) {
        istringstream ss((char*)sqlite3_column_text(statement, 0));
        ss >> target;
      }
    }
  }
}

void Storage::set_setting(std::string key, std::string val) {
  stringstream ss;
  ss << "INSERT OR REPLACE INTO settings(key,str_val) VALUES ('"
     << key << "','" << val << "');";
  exec(ss.str());
}

void Storage::set_setting(std::string key, int val) {
  stringstream ss;
  ss << "INSERT OR REPLACE INTO settings(key,int_val) VALUES ('"
     << key << "'," << val << ");";
  exec(ss.str());
}

void Storage::set_setting(std::string key, float val) {
  stringstream ss;
  ss << "INSERT OR REPLACE INTO settings(key,float_val) VALUES ('"
     << key << "'," << val << ");";
  exec(ss.str());
}

PersistentSettings::~PersistentSettings() {
  //snapshot();
}

