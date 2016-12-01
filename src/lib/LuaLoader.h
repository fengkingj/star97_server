#ifndef __LUA_LOADER_H__
#define __LUA_LOADER_H__
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <unordered_map>

struct LUA_DATA
{
  int type;
  int i;
  bool b;
  std::string s;
  void Print(){
    if(type ==3){
      std::cout<<i<<std::endl;
    }else if(type==4){
      std::cout<<s<<std::endl;
    }else if(type==1&&b){
      std::cout<<"true"<<std::endl;
    }else if(type==1&&!b){
      std::cout<<"false"<<std::endl;
    }
  }
};

class LTable
{
public:
  LTable();
  ~LTable();
  void Clear();
  std::string GetString(const std::string& _key,const char* _default="");
  int GetInt(const std::string& _key,int _default=0);
  int GetInt(unsigned int _index,int _defalut=0);
  bool GetBool(const std::string& _key,bool _default=0);
  LTable* GetChild(unsigned int _index);
  LTable* GetChild(const std::string& _name);
  void AddData(const std::string& _key,const LUA_DATA& _data);
  void AddTable(LTable& _table);
  bool Empty();
  void SetName(const std::string& _name);
  
  std::vector<int> GetInts();
  std::vector<std::string> GetStrings();
  std::vector<bool> GetBools();
 
  inline unsigned int GetDataCount(){return _datas.size();}
  inline unsigned int GetTableID(){return _table_id;}
  inline unsigned int ChildTableSize(){return _ltables.size();}
  void PrintTable();
private:
  std::vector< std::pair<std::string,LUA_DATA> > _datas;
  std::vector<LTable> _ltables;
  unsigned int _table_id;
  static unsigned int _g_table_id;
  std::string _table_name;
};

class lua_State;
class LuaConfig
{
public:
  LuaConfig();
  ~LuaConfig();
  bool LoadFile(const char* _cfile);
  LTable* GetTable(const char* _key);
  std::string GetString(const char* _key,const char* _default="");
  int GetInt(const char* _key,int _default=0);
  bool GetBool(const char* _key,bool _default=0);
  
  void ClearStack();
private:
  lua_State* _L;
  LTable _data_table;
};

class LuaLoader
{
private:
  struct Loader
  {
	LuaConfig _lc;
	bool _success;
	long long _last_time;
  };
public:
  static LuaConfig* LoadFile(const char* _file,bool& modified);
private:
  LuaLoader(){};
  static LuaLoader* _instance;
  std::unordered_map<std::string,Loader> _lua_loaders;
};

#endif