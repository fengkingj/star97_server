#ifndef __EVENT_CALL_H__
#define __EVENT_CALL_H__

#include <functional>
#include <unordered_map>

#define EVENT_FUNCTION(__selector__,__target__)\
  std::bind(&__selector__,__target__,std::placeholders::_1,std::placeholders::_2)

class PackageHead;
class SimpleQueue;
class EventCall
{
public:
  EventCall();
protected:
  void InitCall(SimpleQueue* _queue,int _blocktime=-1);
  
  void RegCall(int _msgtype,std::function<int(PackageHead*,EventCall*)> _cb);
  
  virtual void Dispatch();
  
  virtual int CallBackUnknownPackage(PackageHead* _package,EventCall* _event){return 0;};
protected:
  int _block_time;
private:
  SimpleQueue* _pMsgQueue;
  std::unordered_map<int,std::function<int(PackageHead*,EventCall*)> > _mapCalls;
};

#endif
