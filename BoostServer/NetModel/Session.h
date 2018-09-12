#ifndef __BOOST_SESSION_H__
#define __BOOST_SESSION_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <deque>
#include "StreamNode.h"

// 会话类
class BoostSession : public boost::enable_shared_from_this<BoostSession>
{
public:
	// 
	typedef	boost::shared_ptr<StreamNode>	streamnode_ptr;
	BoostSession(boost::asio::io_service& _ioService);
	virtual ~BoostSession(void);

	void start(void);

	// socket 实例
	boost::asio::ip::tcp::socket& socket(void);
	const boost::posix_time::ptime &getLiveTime();
	void updateLive();
	void write_msg(const char * msg, unsigned int nMsgId,  unsigned int nLen);
private:
	// 完成数据传输后触发的收尾工作
	bool done_handler(const boost::system::error_code& _error);
	// 读取成功后触发的函数
	void read_handler(const boost::system::error_code& _error, size_t _readSize);
	// 写入完成后触发的函数
	void write_handler(const boost::system::error_code& _error, size_t _writeSize);
	
	void async_send();
	unsigned int  getReadLen();
	//std::string  getReadData(int nDataLen = 0);
	unsigned int getReadData(char* pData, int nRead);
	bool getAvailableNode(streamnode_ptr & nodeptr);
	bool addAvailableNode(const streamnode_ptr & nodeptr);
private:
	// 临时信息缓冲区
	char m_cData[BUFFERSIZE];
	std::string currentMsg_;
	// 数据总数量
	int sumSize_;
	// 单个数据包大小
	unsigned int maxSize_;
	// socket句柄
	boost::asio::ip::tcp::socket m_socket;
	// 读取数据缓冲队列
	std::deque<streamnode_ptr> m_pInPutQue;
	// 发送数据缓冲队列
	std::deque<streamnode_ptr> m_pOutPutQue;
    //备用队列
	std::deque<streamnode_ptr> m_pAvailableQue;
	//是否发送完
	bool m_bPendingSend;
	//是否接受完
	bool m_bPendingRecv;
	//消息头
	MsgHeadData m_msgHead;
	//心跳时间
	boost::posix_time::ptime  m_nAliveTime;
};

typedef	boost::shared_ptr<BoostSession>	session_ptr;
typedef boost::weak_ptr<BoostSession> weak_session_ptr;

#endif //__BOOST_SESSION_H__