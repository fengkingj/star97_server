#ifndef __STAR_97_GAME_H__
#define __STAR_97_GAME_H__

#include "../Base/GameService.h"

class Star97Game:public GameService
{
public:
	Star97Game();
	virtual~ Star97Game();
	void Init(int _id,int _type,const char* _name);
	
private:
	PlayerNode* GetFreeNode();
};

#endif