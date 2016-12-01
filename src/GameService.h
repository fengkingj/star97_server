#ifndef __GAME_SERVICE_H__
#define __GAME_SERVICE_H__

#include "Service.h"
#include "lib/NodeManage.h"
#include "PlayerNode.h"

/*
 * Game service,All game logic here
 * Each service inherit thread,Careful using static members
 * @phil
 */

class GameService:public Service
{
public:
	GameService();
	virtual~ GameService();
	
	virtual void Init(int _id,int _type,const char* _name);
	virtual int NodeNum(){return all_players.FreeSize()+all_players.NodeSize();}
private:
	int Thr_UserDisconnect(PackageHead* _package,EventCall* _room);
	int Rad_UserInfoRes(PackageHead* _package,EventCall* _room);
private:
	virtual void OneSecondCall();
	void UpdateOnline(int _login,int _num);
private:
	NodeManage<PlayerNode> all_players;
	int second_count;
};

#endif