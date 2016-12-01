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

void Monitor::MonitorLog()
{
	_log(_ERROR,"Monitor","@[System] Malloc=%lld Free=%lld Con=%lld Discon=%lld MaxFD=%d Online=%d Node=%u",
		 malloc_cnt,free_cnt,connect_cnt,disconnect_cnt,max_fd,online_num,node_num);
	
	vector<Service*> services = ServiceManage::GetService(ST_GAME);
	for(size_t i=0;i<services.size();++i)
	{
		Service* s = services[i];
		_log(_ERROR,"Monitor","@[%s] Online=%d Queue=%d Node=%u",
		  s->ServiceName(),s->total_online,s->QueueSize(),s->NodeNum());
	}
}
