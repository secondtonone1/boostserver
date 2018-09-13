#ifndef __STREAM_NODE_H__
#define __STREAM_NODE_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#define BUFFERSIZE 1024
#define MAXAVAILABLE 50
#define MAXMSGID 1700
#define HEADSIZE 4 // msgid 2 ×Ö½Ú + msglen 2 ×Ö½Ú

class StreamNode:public boost::enable_shared_from_this<StreamNode>
{
public:
	StreamNode(char * msg, int nLen);
	~StreamNode();
	char * getMsgData(void);
	int getRemain();
	int getOffSet();
	void resetOffset(int offsetAdd);
	void cleardata();
	void copydata(char * msg, int nLen);
	boost::shared_ptr<StreamNode> getself();
private:

	char* m_pData;
	int m_nLen;
	int m_nOffSet;
};

#endif //__STREAM_NODE_H__