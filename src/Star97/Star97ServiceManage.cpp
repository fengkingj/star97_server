#include "Star97ServiceManage.h"
#include "../Base/SimpleQueue.h"
#include "../Base/ClientConnection.h"
#include "../Base/Config.h"
#include "Star97Gate.h"
#include "Star97Game.h"

Star97ServiceManage::Star97ServiceManage()
{

}
Star97ServiceManage::~Star97ServiceManage()
{

}

void Star97ServiceManage::StartGateService()
{
	if(gate_service) return;
	
	socket_msg_queue = new SimpleQueue("socket_receive queue",1000);
	reply_client_queue = new SimpleQueue("socket_reply queue",1000);
	  
	ClientConnection* cc = new ClientConnection(3000);
	cc->Ini(socket_msg_queue,reply_client_queue,Server::Inst()->server_port,Server::Inst()->heart_time);
	cc->Start();
	
	gate_service = new Star97Gate();
	gate_service->Init("star97_gate",cc);
	gate_service->Start();
}
void Star97ServiceManage::StartGameService()
{
	if(!game_service.empty()) return;
	for(int i=1;i<=Server::Inst()->work_thread_num;++i)
	{
		Service* pService = new Star97Game();
		char cName[32]={0};
		sprintf(cName,"star97_game_%d",i);
		pService->Init(i,ST_GAME,cName);
		game_service.push_back(pService);
		pService->Start();
	}
}
