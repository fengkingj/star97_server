#ifndef __STAR_97_NODE_H__
#define __STAR_97_NODE_H__
#include "../Base/PlayerNode.h"

class Star97Node:public PlayerNode
{
public:
	virtual~ Star97Node();
	virtual void Reset();
	virtual bool JudgeKickOut();
public:
	
};

#endif