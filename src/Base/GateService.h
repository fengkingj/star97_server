#ifndef __GATE_SERVICE_H__
#define __GATE_SERVICE_H__

#include <unordered_map>
#include <vector>
#include <list>
#include "thread.h"
#include "NodeManage.h"
#include "EventCall.h"
#include "Scheduler.h"

/*
 *主线程-负责用户的认证和消息的分发
 *@phil
 */

class SocketNode
{
public:
	void Reset();
public:
	int m_iUserID;
	int m_iSocketFD;
	int m_iServiceID;
	char m_szIP[20];
	char m_szToken[32];
};

class SimpleQueue;
class ClientConnection;
class Service;
class UserRegMsg;
class PackageHead;
class GateService:public Thread,public EventCall
{
public:
	GateService();
	virtual~ GateService();
	virtual void Init(const char* _name,ClientConnection* _connection);
	
protected:
	virtual int CallBackUnknownPackage(PackageHead* _package,EventCall* _event);
	virtual int Thr_SocketConnect(PackageHead* _package,EventCall* _room);
	virtual int Thr_SocketDisconnect(PackageHead* _package,EventCall* _room);
	
private:	
	int	Run();
	void UpdateServerRoomInfo();
	void SendServerInfoToCServer();
	void SendAuthenRes(int _fd,char _result);
	void UpdateCServerPlayerNum();
	void QueryCenterServerFromRoomServer();
	void ReturnSocketNode(SocketNode* _node);
	
private:
	int Thr_AddRadiusSuccess(PackageHead* _package,EventCall* _room);
	
	int Rad_RoomInfoRes(PackageHead* _package,EventCall* _room);
	int Rad_ServerInfoRes(PackageHead* _package,EventCall* _room);
	int Rad_UpdateCenterServerInfo(PackageHead* _package,EventCall* _room);
	int Rad_UserInfoRes(PackageHead* _package,EventCall* _room);
	int Rad_GetCenterServerMsg(PackageHead* _package,EventCall* _room);
		
	int Cli_UserAuthenReq(PackageHead* _package,EventCall* _room);

protected:
	char _service_name[32];
	ClientConnection* _client_connection;
	NodeManage<SocketNode> _nodeManage;
	std::unordered_map<int,SocketNode*> _nodeManage_uid;
	
	Scheduler scheduler;
};

#endif
