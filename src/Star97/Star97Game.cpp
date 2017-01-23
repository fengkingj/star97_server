#include "Star97Game.h"

Star97Game::Star97Game()
{

}
Star97Game::~Star97Game()
{

}
void Star97Game::Init(int _id, int _type, const char* _name)
{
	GameService::Init(_id, _type, _name);
}
PlayerNode* Star97Game::GetFreeNode()
{
	if(all_players.FreeSize() > 0)
	{
		return all_players.GetFreeNode();
	}
	else
	{
		PlayerNode* p = new PlayerNode();
		p->Reset();
		return p;
	}
}

