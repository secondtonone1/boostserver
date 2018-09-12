#ifndef __BOOST_SERVER_H__
#define __BOOST_SERVER_H__
#include <string.h>  
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "Session.h"
class BoostServer {
private:
	// 保存会话指针
	
public:
	BoostServer(boost::asio::io_service & ioService, boost::asio::ip::tcp::endpoint & endpoint);
	virtual ~BoostServer(void);
	// 服务器开始监听
	void start(void);
	// ioservice run
	void run(void);
private:
	// 新连接回调函数
	void accept_handler(session_ptr chatSession, const boost::system::error_code& errorcode);
	//处理异常
	void handleExpConn();
private:
	boost::asio::io_service &m_ioservice;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::asio::deadline_timer m_timer;
	std::list<weak_session_ptr > m_listweaksession;
};


#endif //__BOOST_SERVER_H__