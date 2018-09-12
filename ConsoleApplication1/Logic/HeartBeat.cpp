#include "HeartBeat.h"
#include <iostream>

void HeartBeat::HandleHeartBeat(const unsigned int &nMsgId, const std::string &strMsg, const weak_session_ptr &pSession)
{
	if(pSession.expired())
		return;
	pSession.lock()->updateLive();
    std::string strnotify = "Update HeartBeat Success !!!";
	std::cout <<"send msg: "<< strnotify <<std::endl;
	pSession.lock()->write_msg(strnotify.c_str(), nMsgId, strnotify.length());
}