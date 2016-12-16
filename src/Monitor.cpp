#include "Monitor.h"
#include "log.h"
#include "Service.h"
#include "ServiceManage.h"
#include <vector>

using namespace std;

long long Monitor::malloc_cnt = 0;
long long Monitor::free_cnt = 0;
long long Monitor::connect_cnt = 0;
long long Monitor::disconnect_cnt = 0;
int Monitor::max_fd = 0;
int Monitor::online_num = 0;	
unsigned int Monitor::node_num = 0;

long long Monitor::queue1_size = 0;
long long Monitor::queue1_count = 0;
long long Monitor::queue2_size = 0;
long long Monitor::queue2_count = 0;
long long Monitor::queue3_size = 0;
long long Monitor::queue3_count = 0;

void Monitor::MonitorLog()
{
	//system
	_log(_ERROR,"Monitor","@[System] Malloc=%lld Free=%lld Con=%lld Discon=%lld MaxFD=%d Online=%d Node=%u",
		 malloc_cnt,free_cnt,connect_cnt,disconnect_cnt,max_fd,online_num,node_num);
	
	//service
	vector<Service*> services = ServiceManage::GetService(ST_GAME);
	for(size_t i=0;i<services.size();++i)
	{
		Service* s = services[i];
		_log(_ERROR,"Monitor","@[%s] Online=%d Queue=%d Node=%u",s->ServiceName(),s->total_online,s->QueueStress(),s->NodeNum());
	}
	
	//global queue
	int q1 = queue1_size/queue1_count;
	int q2 = queue2_size/queue2_count;
	int q3 = queue3_size/queue3_count;
	_log(_ERROR,"Monitor","@[Queue] SocketMsg=%d SocketReq=%d RadiusReq=%d",q1,q2,q3);
	queue1_size = 0;queue1_count = 1;
	queue2_size = 0;queue2_count = 1;
	queue3_size = 0;queue3_count = 1;
}
void Monitor::MonitorQueue(int _size1, int _size2, int _size3)
{
	queue1_size += _size1;++queue1_count;
	queue2_size += _size2;++queue2_count;
	queue3_size += _size3;++queue3_count;
}

