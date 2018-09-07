#include "Server.h"
#include <boost/bind.hpp>
#include "Session.h"
#include <iostream>
using namespace std;
BoostServer::BoostServer(boost::asio::io_service &_ioService, boost::asio::ip::tcp::endpoint &_endpoint)
	: m_ioservice(_ioService), m_acceptor(_ioService, _endpoint),m_timer(_ioService,boost::posix_time::minutes(3)) {
		m_timer.async_wait(boost::bind(&BoostServer::handleExpConn,this));
		start();
}

BoostServer::~BoostServer(void) {
}

void BoostServer::start(void) {
	session_ptr	new_chat_session(new BoostSession(m_ioservice));
	m_acceptor.async_accept(new_chat_session->socket(),
		boost::bind(&BoostServer::accept_handler, this, new_chat_session,
		boost::asio::placeholders::error));
}

void BoostServer::run(void) {
	m_ioservice.run();
}

void BoostServer::accept_handler(session_ptr _chatSession, const boost::system::error_code& _error) {
	if (!_error && _chatSession) {
		try {
			cout << "accept connection from "<< _chatSession->socket().remote_endpoint().address().to_string()<<endl;
			_chatSession->start();
			start();
		}
		catch (...) {
			return;
		}
	}
	else
	{
		
	}
}

//处理异常链接
void BoostServer::handleExpConn()
{
	
}
