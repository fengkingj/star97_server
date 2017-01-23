#ifndef __STAR_97_CONFIG_H__
#define __STAR_97_CONFIG_H__
#include <vector>

#define ALL_GRID_NUM 9
#define ALL_PIC_NUM 9
#define QUANPAN_TYPE_NUM 11
enum GridPic
{
	PIC_Seven		= 1,
	PIC_Blue_Bar	= 2,
	PIC_Red_Bar		= 3,
	PIC_Green_Bar	= 4,
	PIC_Bell		= 5,
	PIC_Watermelon	= 6,
	PIC_Grape		= 7,
	PIC_Orange		= 8,
	PIC_Cherry		= 9,
};
struct LineDef
{	
	int first;
	int second;
	int third;
};
struct LineBet
{
	int first;
	int second;
	int third;
	int bet;
};

class Star97Config
{
public:
	static Star97Config* Ins();
	void LoadConfig();
private:
	Star97Config();
	static Star97Config* _instance;	
private:
	GridPic grid_pic_def[ALL_PIC_NUM];
	int grid_rate[ALL_GRID_NUM][ALL_PIC_NUM];
	int num_seven_bet[ALL_GRID_NUM];
	int quanpan_rate[QUANPAN_TYPE_NUM];
	int quanpan_bet[QUANPAN_TYPE_NUM];
	std::vector<LineDef> all_lines;
	std::vector<LineBet> line_bets;
};

#endif