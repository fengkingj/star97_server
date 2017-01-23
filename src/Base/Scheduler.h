#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include <vector>
#include <list>
#include <functional>

/*
 * 定时器,精确到秒
 * 不想妄想此类可以多线程调用
 */
class Scheduler
{	
public:
	typedef std::function<void()> TimerCallback;
	Scheduler();
	virtual~ Scheduler();
	void Update();
	inline time_t Now(){return tm_now;}
	unsigned int OnceSchedule(TimerCallback _callback,unsigned int _delay = 0);
	unsigned int RepeatSchedule(TimerCallback _callback,unsigned int rate,unsigned int _delay = 0);
	bool RemoveSchedule(unsigned int _scheduleid);
protected:
	struct Timer
	{
		unsigned int unique_id;
		unsigned int repeat_rate;
		TimerCallback schedule_callback;
		time_t start_time;
		time_t last_update_time;
	};
private:
	time_t tm_now;
	time_t tm_last;
	std::vector<Timer> repeating_timers;
	std::list<Timer> once_timers;
	unsigned int id_count;
};

#endif