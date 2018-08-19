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

private:
	// 完成数据传输后触发的收尾工作
	void done_handler(const boost::system::error_code& _error);
	// 读取成功后触发的函数
	void read_handler(const boost::system::error_code& _error, size_t _readSize);
	// 写入完成后触发的函数
	void write_handler(const boost::system::error_code& _error, size_t _writeSize);
	void write_msg(char * msg);
	void async_send();

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
	bool m_bPendingSend;
};

#endif //__BOOST_SESSION_H__