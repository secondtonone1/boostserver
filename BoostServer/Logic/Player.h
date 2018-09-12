#ifndef __MSG_PLAYER_H__
#define __MSG_PLAYER_H__
#include "Singleton.h"
#include "../NetModel/Session.h"
#include "MsgDefine.h"
//A class that handles logic
class Player
{
public:
	Player();
	~Player();
private:
	uint64 m_nPlayerUid;

};

#endif //__MSG_PLAYER_H__