#include "EventCall.h"
#include "lib/SimpleQueue.h"
#include "Package.h"
#include "proto_body.h"

using namespace std;

EventCall::EventCall()
{
}
void EventCall::InitCall(SimpleQueue* _queue,int _blocktime)
{
	_pMsgQueue = _queue;
	_block_time = _blocktime;
}

void EventCall::RegCall(int _msgtype, function<int(PackageHead*,EventCall*)> _cb)
{
	_mapCalls[_msgtype] = _cb;
}
void EventCall::Dispatch()
{
	int iLen=0;
	PackageHead* package=(PackageHead*)_pMsgQueue->DeQueue(iLen,_block_time);
	if(!package)
	{
		return;
	}
	
	MsgHead* mHead = (MsgHead*)((char*)package+sizeof(PackageHead));
	std::unordered_map<int,function<int(PackageHead*,EventCall*)> >::iterator it=_mapCalls.find(mHead->cMsgType);
	bool bfree = true;
	if(it!=_mapCalls.end())
	{
		if((it->second)(package,this) == DONOT_FREE)
		{
			bfree = false;
		}
	}
	else if(CallBackUnknownPackage(package,this) == DONOT_FREE)
	{
		bfree = false;
	}
	if(bfree)
	{
		FREE(package);
	}
}
