#include "PlayerNode.h"
#include "log.h"
#include "string.h"

void PlayerNode::Reset()
{
	iUserID = 0;
	iSocketFD = 0;
	memset(cUserName,0,sizeof(cUserName));
	memset(cNickName,0,sizeof(cNickName));
	iAgentID = 0;
	iMoney = 0;
	iCZContribution = 0;
	iDiamond = 0;
	iUsedDiamond = 0;
	iVipLev = 0;
	iTicket = 0;
	iTicketContribution = 0;
}
