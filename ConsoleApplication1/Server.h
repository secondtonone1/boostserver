#ifndef __BOOST_SERVER_H__
#define __BOOST_SERVER_H__
#include <string.h>  
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
class BoostSession;
class BoostServer {
private:
	// 会话 - 智能指针
	typedef	boost::shared_ptr<BoostSession>	session_ptr;
	typedef boost::weak_ptr<BoostSession> weak_session_ptr;
public:
	BoostServer(boost::asio::io_service & ioService, boost::asio::ip::tcp::endpoint & endpoint);
	virtual ~BoostServer(void);
	// 监听
	void start(void);
	// 异步
	void run(void);
private:
	// 会话启动
	void accept_handler(session_ptr chatSession, const boost::system::error_code& errorcode);
	//处理异常链接
	void handleExpConn();
private:
	boost::asio::deadline_timer m_timer;
	boost::asio::io_service &m_ioservice;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::list<weak_session_ptr > m_listweaksession;
};


#endif //__BOOST_SERVER_H__