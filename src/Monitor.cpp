#include "Monitor.h"
#include "log.h"

long long Monitor::_malloc_cnt=0;
long long Monitor::_free_cnt=0;
long long Monitor::_connect_cnt=0;
long long Monitor::_disconnect_cnt=0;
long long Monitor::_snode_get_cnt = 0; 
long long Monitor::_snode_return_cnt = 0; 
unsigned int Monitor::_snode_num = 0;
long long Monitor::_cnode_get_cnt = 0;
long long Monitor::_cnode_return_cnt = 0;
unsigned int Monitor::_cnode_num = 0;
int Monitor::_max_fd=0;

long long Monitor::_gnode_get_cnt = 0;
long long Monitor::_gnode_return_cnt = 0;
unsigned int Monitor::_gnode_num = 0;

void Monitor::MonitorLog()
{
	_log(_ERROR,"Monitor","@@@ Malloc=%lld Free=%lld Connect=%lld Disconnect=%lld MaxFD=%d",_malloc_cnt,_free_cnt,_connect_cnt,_disconnect_cnt,_max_fd);
	_log(_ERROR,"Monitor","@@@ SNode[num=%u get=%lld ret=%lld] CNode[num=%u get=%lld ret=%lld] GNode[num=%u get=%lld ret=%lld]",_snode_num,_snode_get_cnt,_snode_return_cnt,
		 _cnode_num,_cnode_get_cnt,_cnode_return_cnt,_gnode_num,_gnode_get_cnt,_gnode_return_cnt);
}
