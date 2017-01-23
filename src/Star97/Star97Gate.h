#ifndef __START_97_GATE_H__
#define __START_97_GATE_H__

#include "../Base/GateService.h"

class Star97Gate:public GateService
{
public:
	Star97Gate();
	virtual~ Star97Gate();
	void Init(const char* _name,ClientConnection* _connection);
private:
	
};

#endif