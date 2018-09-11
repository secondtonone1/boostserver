#ifndef __MSG_HANDLER_H__
#define __MSG_HANDLER_H__
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include "Singleton.h"
typedef boost::function<void (unsigned int, std::string )> fuctioncallback;
class MsgHandler: public Singleton<MsgHandler>
{
public:
	void RegisterMsg(unsigned int nMsgId,  fuctioncallback fcallback)
	{
		m_mapCallBack[nMsgId] = fcallback;
	}

	void HandleMsg(unsigned int nMsgId, std::string strData)
	{
			auto iterCallBack = m_mapCallBack.find(nMsgId);
			if(iterCallBack == m_mapCallBack.end())
				return;
			iterCallBack->second(nMsgId,strData);
	}

private:
	boost::unordered_map<unsigned int , fuctioncallback> m_mapCallBack;

};


#endif//__MSG_HANDLER_H__