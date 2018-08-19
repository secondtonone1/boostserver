#include "Session.h"
#include <boost/bind.hpp>
#include <string.h>
BoostSession::BoostSession(boost::asio::io_service& _ioService)
	:m_socket(_ioService) {
		memset(m_cData, 0, BUFFERSIZE);
		m_bPendingSend = false;
}

BoostSession::~BoostSession(void)
{
}

void BoostSession::start(void) {
	memset(m_cData,0,BUFFERSIZE);
	m_socket.async_read_some(boost::asio::buffer(m_cData, BUFFERSIZE-1),
		boost::bind(&BoostSession::read_handler, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket& BoostSession::socket(void) {
	return m_socket;
}

// 完成数据传输
void BoostSession::done_handler(const boost::system::error_code& _error) {
	if (_error) {
		return;
	}

	//配合序列化协议，解析取出节点数据，封装回包
	//这里先不做解析处理，先把收到的数据回包给客户端
	if(m_pInPutQue.empty())
		return;
	
	while(!m_pInPutQue.empty())
	{
		char* msgtosend = m_pInPutQue.front()->getMsgData();
		write_msg(msgtosend);
		m_pInPutQue.pop_front();
	}
	write_msg("HelloWorld!!!");
}

void BoostSession::read_handler(const boost::system::error_code& _error, size_t _readSize) {
	if (_error) {
		return;
	}
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(m_cData);
	m_pInPutQue.push_back(nodePtr);
	done_handler(_error);
	start();
}
void BoostSession::write_handler(const boost::system::error_code& _error, size_t _writeSize) {
	if (_error) {
		return;
	}
	if(m_pOutPutQue.empty())
	{
		   m_bPendingSend = false;
		   return;
	}
	std::cout << "Send Msg Success:  "<< std::string(m_pOutPutQue.front()->getMsgData()+m_pOutPutQue.front()->getOffSet(),_writeSize)
		<<std::endl;
	m_pOutPutQue.front()->resetOffset(_writeSize);
	async_send();
}

void BoostSession::write_msg(char * msg)
{
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(msg);
	m_pOutPutQue.push_back(nodePtr);
	if(!m_bPendingSend)
	{
		m_bPendingSend = true;
		async_send();
	}
}

void BoostSession::async_send()
{
	while(!m_pOutPutQue.empty())
	{
 		if(m_pOutPutQue.front()->getRemain() == 0)
		{
			m_pOutPutQue.pop_front();
			continue;
		}
		//找到非空节点
		break;
	}
	if(m_pOutPutQue.empty())
	{
		m_bPendingSend = false;
		return;
	}
	streamnode_ptr &frontNode = m_pOutPutQue.front();
	boost::asio::async_write(m_socket,
		boost::asio::buffer(frontNode->getMsgData()+frontNode->getOffSet(), frontNode->getRemain()),
		boost::bind(&BoostSession::write_handler, this,
		boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
}