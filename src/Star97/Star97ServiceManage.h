#ifndef __STAR97_SERVICE_MANAGE_H__
#define __STAR97_SERVICE_MANAGE_H__

#include "../Base/ServiceManage.h"

class Star97ServiceManage:public ServiceManage
{
public:
	Star97ServiceManage();
	virtual~ Star97ServiceManage();
	
	virtual void StartGateService();
	virtual void StartGameService();
};


#endif