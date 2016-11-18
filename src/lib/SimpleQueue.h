#ifndef __SIMPLE_QUEUE_H__
#define __SIMPLE_QUEUE_H__

#include <pthread.h>

struct QueItem
{
	void* _data;
	int _len;
	QueItem* _next;
	QueItem(){
		_data = 0;
		_len = 0;
		_next = 0;
	}
};
class SimpleQueue
{
public:
  SimpleQueue(const char* _name,int _warning_size);
  virtual ~SimpleQueue();
  void EnQueue(void* _data,int _len);
  void* DeQueue(int& _len,int _time=-1);
private:
  char _queue_name[32];
  int _warning_count;
  int _size;
  QueItem* _head;
  QueItem* _tail;
  pthread_cond_t cond_put;
  pthread_mutex_t  mt_lock;
};

#endif
