#include "LuaLoader.h"
#include "log.h"
#include <iostream>
extern "C"
{
  #include        "lua.h"
  #include        "lualib.h"
  #include        "lauxlib.h"
}

using namespace std;
LuaConfig::LuaConfig()
{
	_L=NULL;
}
LuaConfig::~LuaConfig()
{
	if(_L)
	{
		lua_close(_L);
		_L=NULL;
	}
}
bool LuaConfig::LoadFile(const char* _cfile)
{ 
	if(!_L)
	{
		_L = luaL_newstate();
		if (!_L) 
		{
			_log(_ERROR,"Lua Error","Start Lua State Error,file[%s]",_cfile);
			return false;
		}
		//luaL_openlibs(_L);
	}
	int error = luaL_dofile(_L,_cfile);
	if (error)
	{
		const char* errstr=lua_tostring(_L,-1);
		_log(_ERROR,"Lua Error","Load Lua File:%s Error:%s",_cfile,errstr);
		return false;
	}
	return true;
}
void ParseTable(lua_State* L,LTable& t)
{
	int nIndex = lua_gettop(L);  // 取 table 索引值
	lua_pushnil(L);  // nil 入栈作为初始 key
	LUA_DATA data;
	while( 0 != lua_next( L, nIndex))
	{
		data.type = lua_type(L, -1);
		LTable tt;
		switch (data.type)
		{
			case LUA_TNUMBER:
				data.i = lua_tointeger(L, -1);
				break;
			case LUA_TBOOLEAN:
				data.b = lua_toboolean(L,-1);
				break;
			case LUA_TNIL:
				break;
			case LUA_TSTRING:
				data.s = lua_tostring(L, -1);
				break;
			case LUA_TTABLE:
				ParseTable(L,tt);
				break;
			default:
				_log(_ERROR,"Lua Error","Not support type:%d",data.type);
				lua_pop(L, 1 );  // 弹出 value，让 key 留在栈顶
				continue;
		}
		char buf[128]={0};
		string key;
		if (lua_type(L, -2) == LUA_TSTRING)
		{
			key = lua_tostring(L, -2);
		}
		else if (lua_isnumber(L, -2))
		{
			int ikey = (int)lua_tonumber(L, -2);
			sprintf(buf, "%d", ikey);
			key = buf;
		}
		//std::cout << "*** key=" << key << std::endl;
		if (data.type != LUA_TTABLE)
		{
			t.AddData(key,data);
		}
		else
		{
			tt.SetName(key);
			t.AddTable(tt);
		}
		lua_pop(L,1);
	}
}
LTable* LuaConfig::GetTable(const char* _key)
{
	_data_table.Clear();
	lua_getglobal(_L,_key);
	if(lua_istable(_L,-1))
	{
		ParseTable(_L,_data_table);
		return &_data_table;
	}
	return NULL;
}
bool LuaConfig::GetBool(const char* _key, bool _default)
{
  lua_getglobal(_L,_key);
  if(lua_isboolean(_L,-1))
  {   
    _default = lua_toboolean(_L,-1);
	lua_pop(_L,-1);
  }
  return _default;
}
int LuaConfig::GetInt(const char* _key, int _default)
{
  lua_getglobal(_L,_key);
  if(lua_isinteger(_L,-1))
  {
    _default=(int)lua_tointeger(_L,-1);
	lua_pop(_L,-1);
  }
  return _default;
}
string LuaConfig::GetString(const char* _key, const char* _default)
{
  string res(_default);
  lua_getglobal(_L,_key);
  if(lua_isstring(_L,-1))
  {
    res=lua_tostring(_L,-1);
	lua_pop(_L,-1);
  }
  return res;
}
void LuaConfig::ClearStack()
{
	lua_settop(_L,0);
	_data_table.Clear();
}

/////////////////////////////////////////////////////////////////////
unsigned int LTable::_g_table_id = 0;
LTable::LTable()
{
 
    _g_table_id++;
    _table_id = _g_table_id;
}
LTable::~LTable()
{
 
 
}
void LTable::AddData(const string& _key, const LUA_DATA& _data)
{
  //cout<<" >>Table:"<<_table_id<<" AddData:"<<_key<<endl;
  _datas.push_back(make_pair(_key,_data));
}
void LTable::AddTable(LTable& _table)
{
  //cout<<"Table:"<<_table_id<<" AddTable:"<<_table._table_id<<endl;
  _ltables.push_back(_table);
}
void LTable::Clear()
{
  _datas.clear();
  _ltables.clear();
}
bool LTable::GetBool(const string& _key, bool _default)
{
  for(size_t i=0;i<_datas.size();++i)
  {
    if(_key == _datas[i].first)
    {
      return _datas[i].second.b;
    }
  }
  return _default;
}
int LTable::GetInt(const string& _key, int _default)
{
  for(size_t i=0;i<_datas.size();++i)
  {
    if(_key == _datas[i].first)
    {
      return _datas[i].second.i;
    }
  }
  return _default;
}
string LTable::GetString(const string& _key, const char* _default)
{
  for(size_t i=0;i<_datas.size();++i)
  {
    if(_key == _datas[i].first)
    {
      return _datas[i].second.s;
    }
  }
  return _default;
}
LTable* LTable::GetChild(unsigned int _index)
{
  if(_index<_ltables.size())
  {
    return &(_ltables[_index]);
  }
  return NULL;
}
LTable* LTable::GetChild(const string& _name)
{
	for(size_t i=0;i<_ltables.size();++i)
	{
		if(_ltables[i]._table_name == _name)
		{
			return &(_ltables[i]);
		}
	}
	return NULL;
}

bool LTable::Empty()
{
  if(_ltables.empty() && _datas.empty())
  {
    return true;
  }
  return false;
}
void LTable::PrintTable()
{
  cout<<"======Table:"<<_table_id<<endl;
  for(size_t i=0;i<_datas.size();++i)
  {
    cout<<" key:"<<_datas[i].first<<" value:";
    _datas[i].second.Print();
    cout<<endl;
  }
  for(size_t i=0;i<_ltables.size();++i)
  {
    _ltables[i].PrintTable();
  }
}
void LTable::SetName(const string& _name)
{
	_table_name=_name;
}
int LTable::GetInt(unsigned int _index, int _defalut)
{
	if(_index<_datas.size())
	{
		if(_datas[_index].second.type==LUA_TNUMBER)
		{
			return _datas[_index].second.i;
		}
	}
	return _defalut;
}
vector<int> LTable::GetInts()
{
	vector<int> res;
	for(size_t i=0;i<_datas.size();++i)
	{
		if(_datas[i].second.type==LUA_TNUMBER)
		{
			res.push_back(_datas[i].second.i);
		}
	}
	return res;
}
vector<string> LTable::GetStrings()
{
	vector<string> res;
	for(size_t i=0;i<_datas.size();++i)
	{
		if(_datas[i].second.type==LUA_TSTRING)
		{
			res.push_back(_datas[i].second.s);
		}
	}
	return res;
}
vector<bool> LTable::GetBools()
{
	vector<bool> res;
	for(size_t i=0;i<_datas.size();++i)
	{
		if(_datas[i].second.type==LUA_TBOOLEAN)
		{
			res.push_back(_datas[i].second.b);
		}
	}
	return res;
}

#include <sys/stat.h>
#include <unistd.h>

LuaLoader* LuaLoader::_instance = 0;
LuaConfig* LuaLoader::LoadFile(const char* _file)
{
	if(!_instance)
	{
		_instance = new LuaLoader();
	}	
	struct stat buf;
	int res = stat(_file,&buf);
	if(res != 0)
	{
		_log(_ERROR,"LuaLoader","Problem getting file info.File=%s",_file);
		return NULL;
	}
	long long filetime = (long long)buf.st_mtime+(long long)buf.st_ctime;
	string str(_file);
	unordered_map<string,Loader>::iterator it=_instance->_lua_loaders.find(str);
	if(it!=_instance->_lua_loaders.end())
	{
		if(filetime != it->second._last_time)
		{
			it->second._success = it->second._lc.LoadFile(_file);
			it->second._last_time = filetime;
		}
		if(it->second._success)
		{
			return &(it->second._lc);
		}
	}
	else
	{
		Loader* ld = &(_instance->_lua_loaders[str]);
		ld->_last_time = filetime;
		ld->_success = ld->_lc.LoadFile(_file);
		if(ld->_success)
		{
			return &(ld->_lc);
		}
	}
	return 0;
}




