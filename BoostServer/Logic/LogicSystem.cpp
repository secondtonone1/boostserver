#include "LogicSystem.h"
#include <iostream>
#include "MsgDefine.h"
#include <boost/bind.hpp>
LogicSystem::LogicSystem()
{

}

LogicSystem::~LogicSystem()
{

}

void LogicSystem::startup()
{
	REGISTER_MSG();
}