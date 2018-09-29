#ifndef __MSG_BASE64_H__
#define __MSG_BASE64_H__
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include "Singleton.h"
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

class Base64Handler
{
public:
	Base64Handler(){}
	~Base64Handler(){}
	std::string base64_encode(unsigned char const* , unsigned int len);
	std::string base64_decode(std::string const& s);
private:
	
};
typedef Singleton<Base64Handler> Base64HandlerInst;

#endif//__MSG_BASE64_H__

