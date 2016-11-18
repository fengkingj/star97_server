#ifndef __STAR_97_SERVER_H__
#define __STAR_97_SERVER_H__

#include "Service.h"
#include "lib/NodeManage.h"
#include "PlayerNode.h"

/*
 *Each service inherit thread,Careful when you use static members
 */

class Star97Service:public Service
{
public:
	Star97Service();
	virtual~ Star97Service();
	
	virtual void Init(int _id,int _type,const char* _name);
private:
	int Thr_UserDisconnect(PackageHead* _package,EventCall* _room);
	int Rad_UserInfoRes(PackageHead* _package,EventCall* _room);
private:
	virtual void OneSecondCall();
private:
	NodeManage<PlayerNode> all_players;
	int second_count;
};

#endif