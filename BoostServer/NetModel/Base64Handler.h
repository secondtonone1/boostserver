#ifndef __MSG_BASE64HANDLER_H__
#define __MSG_BASE64HANDLER_H__
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include "Singleton.h"
#include "../NetModel/Session.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
using namespace boost::archive::iterators;
typedef boost::function<void (const unsigned int&, const std::string&,  const weak_session_ptr&)> fuctioncallback;

class Base64Handler
{
public:
	Base64Handler(){}
	~Base64Handler(){}
	bool Base64Encode( const string & input, string * output )
	{
		typedef base64_from_binary<transform_width<string::const_iterator, 6, 8>> Base64EncodeIterator;
		stringstream result;
		try {
			copy( Base64EncodeIterator( input.begin() ), Base64EncodeIterator( input.end() ), ostream_iterator<char>( result ) );
		} catch ( ... ) {
			return false;
		}
		size_t equal_count = (3 - input.length() % 3) % 3;
		for ( size_t i = 0; i < equal_count; i++ )
		{
			result.put( '=' );
		}
		*output = result.str();
		return output->empty() == false;
	}

	bool Base64Decode( const string & input, string * output )
	{
		typedef transform_width<binary_from_base64<string::const_iterator>, 8, 6> Base64DecodeIterator;
		stringstream result;
		try {
			copy( Base64DecodeIterator( input.begin() ), Base64DecodeIterator( input.end() ), ostream_iterator<char>( result ) );
		} catch ( ... ) {
			return false;
		}
		*output = result.str();
		return output->empty() == false;
	}
	
private:
	

};
typedef Singleton<Base64Handler> Base64HandlerInst;

#endif//__MSG_BASE64HANDLER_H__

