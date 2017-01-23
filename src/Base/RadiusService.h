#ifndef __RADIUS_SERVICE_H__
#define __RADIUS_SERVICE_H__

#include "thread.h"
#include "TCPClient.h"
#include <vector>
#include <string>
#include <unordered_map>

#define READ_BUFF_SIZE 1024*100
#define WRITE_BUFF_SIZE 1024*100

#define RECONNECT_INTERVAL 10	//重连时间间隔

enum RadiusType
{
	RAD_CENTER = 1,
	RAD_ACCOUNT,
	RAD_ROOM,
	RAD_LOG,
};
struct Radius
{
	RadiusType eType;
	char cName[32];
	char cIP1[20];
	int iPort1;
	char cIP2[20];
	int iPort2;
	int iSocketFD;
	int iReconnectCount;
	bool bAesEncrypt;
};

class SimpleQueue;
class RadiusService:public TCPClient,public Thread
{
public:
	RadiusService();
	virtual~ RadiusService();
	void Init(int _id,const char* _name,SimpleQueue* _request,SimpleQueue* _reply);
	int AddRadius(Radius& radius);
	static int GetRadiusFD(RadiusType rt);
private:
	int	Run();
	void ReconnectRadius(int index);
	void KeepRadiusAlive();
	void ConnectSuccess(int fd,bool aes,bool newcon);
	
	void SendRadiusData(int fd,bool aes,void* data,int len);
	
	virtual void CallbackAddSocketNode(int fd);
	virtual void CallbackTCPReadData(int fd);
	virtual void CallbackDelSocketNode(int fd);
private:
	std::vector<Radius> vec_all_radius;
	static std::unordered_map<int,int> map_radius_fd;
	int radius_id;
	char radius_name[32];
	SimpleQueue* request_queue;
	SimpleQueue* reply_queue;
};

#endif