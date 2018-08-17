#ifndef __STREAM_NODE_H__
#define __STREAM_NODE_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#define BUFFERSIZE 1024
class StreamNode:public boost::enable_shared_from_this<StreamNode>
{
public:
	StreamNode(char * msg);
	~StreamNode();
private:

	char* m_pData;
	int m_nLen;
	int m_nOffSet;
};

#endif //__STREAM_NODE_H__