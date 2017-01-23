#include "Scheduler.h"

using namespace std;
Scheduler::Scheduler()
{
	id_count = 0;
	tm_last = time(0);
}
Scheduler::~Scheduler()
{

}
void Scheduler::Update()
{
	tm_now = time(0);
	if(tm_now <= tm_last) return;
	tm_last = tm_now;
	
	vector<TimerCallback> vecCall;
	std::list<Timer>::iterator it = once_timers.begin();
	for(; it != once_timers.end();)
	{
		if(tm_now >= it->start_time)
		{
			vecCall.push_back(it->schedule_callback);
			once_timers.erase(it++);
		}
		else
		{
			++it;
		}
	}
	for(size_t i=0;i<repeating_timers.size();++i)
	{
		if(tm_now >= repeating_timers[i].start_time)
		{
			if(tm_now - repeating_timers[i].last_update_time >= repeating_timers[i].repeat_rate)
			{
				repeating_timers[i].last_update_time = tm_now;
				vecCall.push_back(repeating_timers[i].schedule_callback);
			}
		}
	}
	for(size_t i=0;i<vecCall.size();++i)
	{
		vecCall[i]();
	}
}
unsigned int Scheduler::OnceSchedule(TimerCallback _callback,unsigned int _delay)
{
	Timer t;
	t.start_time = time(0) + _delay;
	t.schedule_callback = _callback;
	t.unique_id = ++id_count;
	once_timers.push_back(t);
	return t.unique_id;
}
unsigned int Scheduler::RepeatSchedule(TimerCallback _callback,unsigned int rate,unsigned int _delay)
{
	Timer t;
	t.start_time = time(0) + _delay;
	t.last_update_time = 0;
	t.schedule_callback = _callback;
	t.unique_id = ++id_count;
	t.repeat_rate = rate;
	repeating_timers.push_back(t);
	return t.unique_id;
}
bool Scheduler::RemoveSchedule(unsigned int _scheduleid)
{
	for(vector<Timer>::iterator it = repeating_timers.begin();it != repeating_timers.end();++it)
	{
		if(it->unique_id == _scheduleid)
		{
			repeating_timers.erase(it);
			return true;
		}
	}
	for(list<Timer>::iterator it = once_timers.begin();it!=once_timers.end();++it)
	{
		if(it->unique_id == _scheduleid)
		{
			once_timers.erase(it);
			return true;
		}
	}
	return false;
}

