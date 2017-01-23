#include "Star97Config.h"
#include "log.h"
#include "../Base/LuaLoader.h"
#include "../Base/Config.h"
#include <string>
using namespace std;

Star97Config* Star97Config::_instance = 0;
Star97Config::Star97Config()
{
	grid_pic_def[0] = PIC_Seven;
	grid_pic_def[1] = PIC_Blue_Bar;
	grid_pic_def[2] = PIC_Red_Bar;
	grid_pic_def[3] = PIC_Green_Bar;
	grid_pic_def[4] = PIC_Bell;
	grid_pic_def[5] = PIC_Watermelon;
	grid_pic_def[6] = PIC_Grape;
	grid_pic_def[7] = PIC_Orange;
	grid_pic_def[8] = PIC_Cherry;
}
Star97Config* Star97Config::Ins()
{
	if(!_instance)
	{
		_instance = new Star97Config();
	}
	return _instance;
}
GridPic get_grid_picture(std::string& name)
{
	if(name == "seven") return PIC_Seven;
	else if(name == "blue") return PIC_Blue_Bar;
	else if(name == "red") return PIC_Red_Bar;
	else if(name == "green") return PIC_Green_Bar;
	else if(name == "bell") return PIC_Bell;
	else if(name == "watermelon") return PIC_Watermelon;
	else if(name == "grape") return PIC_Grape;
	else if(name == "orange") return PIC_Orange;
	else if(name == "cherry") return PIC_Cherry;
	else{
		_log(_ERROR,"ConfigError","get_grid_picture name[%s]",name.c_str());
		return PIC_Cherry;
	}
}
void Star97Config::LoadConfig()
{
	string file = Server::Inst()->config_data_path+"/game_config.lua";
	if(!LuaLoader::FileModified(file))
	{
		return;
	}
	LuaConfig* lc = LuaLoader::LoadFile(file);
	if(!lc)
	{
		return;
	}
	LTable* t = lc->GetTable("quanpan_bet");
	if(t)
	{
		vector<int> temp = t->GetInts();
		copy(temp.begin(),temp.end(),quanpan_bet);
		_log(_ERROR,"Config","LoadConfig quanpan_bet[%d][%d]...[%d][%d]",quanpan_bet[0],quanpan_bet[1],quanpan_bet[9],quanpan_bet[10]);
	}
	t = lc->GetTable("quanpan_rate");
	if(t)
	{
		vector<int> temp = t->GetInts();
		copy(temp.begin(),temp.end(),quanpan_rate);
		_log(_ERROR,"Config","LoadConfig quanpan_rate[%d][%d]...[%d][%d]",quanpan_rate[0],quanpan_rate[1],quanpan_rate[9],quanpan_rate[10]);
	}
	t = lc->GetTable("seven_mul");
	if(t)
	{
		vector<int> temp = t->GetInts();
		copy(temp.begin(),temp.end(),num_seven_bet);
		_log(_ERROR,"Config","LoadConfig num_seven_bet[%d][%d]...[%d][%d]",num_seven_bet[0],num_seven_bet[1],num_seven_bet[7],num_seven_bet[8]);
	}
	t = lc->GetTable("grid_rate");
	if(t)
	{
		unsigned int num = t->ChildTableSize();
		for(unsigned int i=0;i<num;++i)
		{
			vector<int> temp = t->GetChild(i)->GetInts();
			copy(temp.begin(),temp.end(),grid_rate[i]);
			_log(_ERROR,"Config","LoadConfig grid[%d] rate[%d][%d]...[%d][%d]",i,grid_rate[i][0],grid_rate[i][1],grid_rate[i][7],grid_rate[i][8]);
		}
	}
	lc->ClearStack();
}


