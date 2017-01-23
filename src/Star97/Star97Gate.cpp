#include "Star97Gate.h"

Star97Gate::Star97Gate()
{

}
Star97Gate::~Star97Gate()
{

}
void Star97Gate::Init(const char* _name, ClientConnection* _connection)
{
	GateService::Init(_name, _connection);
}
