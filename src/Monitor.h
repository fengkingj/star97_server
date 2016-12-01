#ifndef __MONITOR_H__
#define __MONITOR_H__

#define MONITOR_LOG_TIME 600

class Monitor
{
public:
  	static void MonitorLog();
	static long long malloc_cnt;
	static long long free_cnt;
	static long long connect_cnt;
	static long long disconnect_cnt;
	static int max_fd;
	static int online_num;	
	static unsigned int node_num;
};
#endif