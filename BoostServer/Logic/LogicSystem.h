#ifndef __MSG_LOGICSYSTEM_H__
#define __MSG_LOGICSYSTEM_H__
#include "../NetModel/Singleton.h"
#include "../NetModel/Session.h"
#include "Player.h"
#include <boost/unordered_map.hpp>
//A class that handles logic
class LogicSystem: public Singleton<LogicSystem>
{
public:
	LogicSystem();
	~LogicSystem();
	void startup();
private:
	boost::unordered_map<uint64, Player> m_mapPlayer;
};

#endif //__MSG_LOGICSYSTEM_H__