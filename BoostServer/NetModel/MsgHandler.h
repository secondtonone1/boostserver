#ifndef __MSG_HANDLER_H__
#define __MSG_HANDLER_H__
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include "Singleton.h"
#include "../NetModel/Session.h"
typedef boost::function<void (const unsigned int&, const std::string&,  const weak_session_ptr&)> fuctioncallback;

class MsgHandler
{
public:
	MsgHandler(){}
	~MsgHandler(){}

	void RegisterMsg(unsigned int nMsgId,  fuctioncallback fcallback)
	{
		m_mapCallBack[nMsgId] = fcallback;
	}

	void HandleMsg(const unsigned int &nMsgId, const std::string &strData, const weak_session_ptr &pSession)
	{
		auto iterCallBack = m_mapCallBack.find(nMsgId);
		if(iterCallBack == m_mapCallBack.end())
			return;
		iterCallBack->second(nMsgId,strData,pSession);
	}
private:
	boost::unordered_map<unsigned int , fuctioncallback> m_mapCallBack;

};
typedef Singleton<MsgHandler> MsgHandlerInst;

#endif//__MSG_HANDLER_H__