#ifndef __STREAM_NODE_H__
#define __STREAM_NODE_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#define BUFFERSIZE 1024
#define HEADSIZE 4
#define MSGIDSIZE 4
#define MAXMSGID 2048
class StreamNode:public boost::enable_shared_from_this<StreamNode>
{
public:
	StreamNode(char * msg, int nLen);
	~StreamNode();
	char * getMsgData(void);
	int getRemain();
	int getOffSet();
	void resetOffset(int offsetAdd);
private:

	char* m_pData;
	int m_nLen;
	int m_nOffSet;
};

#endif //__STREAM_NODE_H__