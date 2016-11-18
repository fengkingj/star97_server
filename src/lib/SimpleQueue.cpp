#include "SimpleQueue.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/errno.h>

SimpleQueue::SimpleQueue(const char* _name, int _warning_size)
{
	strcpy(_queue_name,_name);
	_warning_count = _warning_size;
	_head = NULL;
	_tail = NULL;
	_size = 0;
	pthread_cond_init(&cond_put, NULL);
	pthread_mutex_init(&mt_lock, NULL);
}
SimpleQueue::~SimpleQueue()
{
	pthread_mutex_destroy(&mt_lock);
	pthread_cond_destroy(&cond_put);
	if(_head&&_tail)
	{
		while(_head != _tail)
		{
			QueItem* t = _head;
			_head = _head->_next;
			delete t;
		}
		delete _head;
		_head = NULL;
		_tail = NULL;
	}
}

void SimpleQueue::EnQueue(void* _data,int _len)
{
	assert(_data);
	if(_size>_warning_count && _size%(_warning_count/2)==0)
	{
		_log(_ERROR,"SimpleQueue","EnQueue Queue[%s] Large Warning[%d] now[%d]",_queue_name,_warning_count,_size);
	}
	
	pthread_mutex_lock(&mt_lock);//这里锁住防止多个线程同时调用EnQueue函数导致冲突
	QueItem* temp = new QueItem();
	temp->_data = _data;
	temp->_len = _len;
	if(_head == NULL)
	{
		_head = temp;
		_tail = temp;
	}
	else
	{
		_tail->_next=temp;
		_tail=temp;
	}
	_size ++;
	pthread_cond_signal(&cond_put); 
	pthread_mutex_unlock(&mt_lock);
}
void* SimpleQueue::DeQueue(int& _len,int _time)
{
	void* res;
	pthread_mutex_lock(&mt_lock);
	while(_head == NULL)  
	{
		if(_time==-1)//block
		{
			pthread_cond_wait(&cond_put, &mt_lock);  
		}
		else if(_time==0)//return now
		{
			pthread_mutex_unlock(&mt_lock);
			_len = 0;
			return NULL;
		}
		else 
		{
			struct timeval tp;
			struct timespec atime;
			gettimeofday(&tp,NULL);
			atime.tv_sec = tp.tv_sec + _time;
			atime.tv_nsec = 0 ;
			int ret = pthread_cond_timedwait(&cond_put, &mt_lock,&atime);
			if (ret == ETIMEDOUT)
			{
				pthread_mutex_unlock(&mt_lock); 
				_len = 0;
				return NULL;
			}
		}
	}
    if(_head->_next == NULL)
	{
		res = _head->_data;
		_len = _head->_len;
		delete _head;
		_head = NULL;
		_tail = NULL;
	}
	else
	{
		QueItem* d = _head;
		res = d->_data;
		_len = d->_len;
		_head = _head->_next;
		delete d;
	}
	_size --;
	pthread_mutex_unlock(&mt_lock);  
	
	return res;
}