#ifndef __MSG_DEFINE_H__
#define __MSG_DEFINE_H__
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../NetModel/MsgHandler.h"
#include "HeartBeat.h"
typedef unsigned long long  uint64;
typedef unsigned int uint32;
enum MsgIDs
{
	//基本消息
	MsgHeartBeat = 1, //心跳
};


#define REGISTER_MSG()  do{ \
	MsgHandlerInst::instance()->RegisterMsg(MsgHeartBeat, boost::bind( HeartBeat::HandleHeartBeat, _1,_2,_3)); \
	\
	\
}while(0)


#endif//__MSG_DEFINE_H__