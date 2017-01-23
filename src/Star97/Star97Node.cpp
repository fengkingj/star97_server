#include "Star97Node.h"

Star97Node::~Star97Node()
{

}
void Star97Node::Reset()
{
	PlayerNode::Reset();
}
bool Star97Node::JudgeKickOut()
{
	return PlayerNode::JudgeKickOut();
}
