#ifndef __MSG_HEARTBEAT_H__
#define __MSG_HEARTBEAT_H__
#include "../NetModel/Singleton.h"
#include "../NetModel/Session.h"
class HeartBeat
{
public:
	HeartBeat(){}
	~HeartBeat(){}
	static void HandleHeartBeat(const unsigned int &nMsgId, const std::string &strMsg, const weak_session_ptr &pSession);
};

#endif //__MSG_HEARTBEAT_H__