#ifndef __MONITOR_H__
#define __MONITOR_H__

#define MONITOR_LOG_TIME 120

class Monitor
{
public:
  	static void MonitorLog();
	static long long _malloc_cnt;
	static long long _free_cnt;
	static long long _connect_cnt;
	static long long _disconnect_cnt;
	static int _max_fd;
	
	static long long _snode_get_cnt; 
	static long long _snode_return_cnt; 
	static unsigned int _snode_num;
	
	static long long _cnode_get_cnt;
	static long long _cnode_return_cnt;
	static unsigned int _cnode_num;
	
	static long long _gnode_get_cnt;
	static long long _gnode_return_cnt;
	static unsigned int _gnode_num;
};
#endif