#ifndef __MSG_SINGLETON_H__
#define __MSG_SINGLETON_H__
#include <iostream>
#include <boost\shared_ptr.hpp>

template <typename T>
class Singleton
{
public:
	static boost::shared_ptr<T> instance()
	{
		if (_instance == 0)
			_instance = boost::shared_ptr<T>(new T);
		return _instance;
	}

	~Singleton()
	{
		std::cout << "Singleton::~Singleton()" << std::endl;
	}
protected:
	Singleton(const Singleton&){}
	Singleton()
	{
		std::cout << "Singleton::Singleton()" << std::endl;
	}
private:
	Singleton& operator=(const Singleton&){}
	static boost::shared_ptr<T> _instance;
};

template <typename T>
boost::shared_ptr<T> Singleton::_instance = NULL;

#endif//__MSG_SINGLETON_H__