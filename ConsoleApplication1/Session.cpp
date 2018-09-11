#include "Session.h"
#include <boost/bind.hpp>
#include <string.h>
#include <assert.h>
BoostSession::BoostSession(boost::asio::io_service& _ioService)
	:m_socket(_ioService),m_nAliveTime(boost::posix_time::microsec_clock::universal_time()) {
		memset(m_cData, 0, BUFFERSIZE);
		m_bPendingSend = false;
		m_bPendingRecv = false;
		m_msgHead.m_nMsgId =0;
		m_msgHead.m_nMsgLen = 0;
}

BoostSession::~BoostSession(void)
{
	m_pInPutQue.clear();
	m_pOutPutQue.clear();
	m_pAvailableQue.clear();
}

void BoostSession::start(void) {
	memset(m_cData,0,BUFFERSIZE);
	m_socket.async_read_some(boost::asio::buffer(m_cData, BUFFERSIZE),
		boost::bind(&BoostSession::read_handler, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket& BoostSession::socket(void) {
	return m_socket;
}

// 完成数据传输
bool BoostSession::done_handler(const boost::system::error_code& _error) {
	if (_error) {
		return false;
	}
	//配合序列化协议，解析取出节点数据，封装回包
	//这里先不做解析处理，先把收到的数据回包给客户端
	while(!m_pInPutQue.empty())
	{
		int nRemain = m_pInPutQue.front()->getRemain();
		//判断是否读全，如果读全则pop头结点，继续读下一个节点
		if(nRemain == 0)
		{
			m_pInPutQue.pop_front();
			continue;
		}
		//新的包
		if(m_bPendingRecv == false)
		{
			//该节点接收数据小于规定包头大小
			if(getReadLen() < HEADSIZE)
			{
				return true;
			}
			getReadData((char *)&m_msgHead, HEADSIZE);
			m_msgHead.m_nMsgId = ntohl(m_msgHead.m_nMsgId);
			m_msgHead.m_nMsgLen = ntohl(m_msgHead.m_nMsgLen);
		  
			if(m_msgHead.m_nMsgId > MAXMSGID)
				return false;
			if(m_msgHead.m_nMsgLen > BUFFERSIZE)
				return false;
			if(getReadLen() < m_msgHead.m_nMsgLen)
			{
				m_bPendingRecv = true;
				return true;
			}
			//接收完全
			char strMsgData[BUFFERSIZE]={0};
			getReadData(strMsgData,m_msgHead.m_nMsgLen);
			std::cout <<"Receive Data : " <<strMsgData <<std::endl;
			write_msg(strMsgData,m_msgHead.m_nMsgId,m_msgHead.m_nMsgLen);
			continue;
		}
		 //继续上次未收全的接收
		if(getReadLen() <m_msgHead.m_nMsgLen)
			return true;
		//接收完全
		char strMsgData[BUFFERSIZE]={0};
	    getReadData(strMsgData,m_msgHead.m_nMsgLen);
		std::cout <<"Receive Data : " <<strMsgData <<std::endl;
		write_msg(strMsgData,m_msgHead.m_nMsgId,m_msgHead.m_nMsgLen);
		m_bPendingRecv = false;

	}
	return true;
}

int  BoostSession::getReadLen()
{
	int nTotal = 0;
	for( auto i = m_pInPutQue.begin(); i != m_pInPutQue.end(); i++)
	{
		nTotal += i->get()->getRemain();
	}
	return nTotal;
}

int BoostSession::getReadData(char* pData, int nRead)
{
	if(pData == NULL)
		return 0;
	if(nRead == 0)
		return 0;
	int nCur = 0;
	while(m_pInPutQue.empty() == false)
	{
		//节点可读数据大于请求数据
		if(m_pInPutQue.front()->getRemain() >= nRead)
		{
			char * msgData =m_pInPutQue.front()->getMsgData();
			memcpy(pData+nCur,msgData+m_pInPutQue.front()->getOffSet(),nRead);
			nCur+=nRead;
			m_pInPutQue.front()->resetOffset(nRead);
			return nCur;
		}
		//节点可读数据为空
		if(m_pInPutQue.front()->getRemain() == 0)
		{
			m_pInPutQue.pop_front();
			continue;
		}
		//节点有可读数据，且小于请求数据
		char * msgData = m_pInPutQue.front()->getMsgData();
		memcpy(pData+nCur,msgData+m_pInPutQue.front()->getOffSet(),m_pInPutQue.front()->getRemain());
		nRead-=m_pInPutQue.front()->getRemain();
		nCur+=m_pInPutQue.front()->getRemain();
		m_pInPutQue.pop_front();
	}
	return nCur;
}



	const boost::posix_time::ptime &BoostSession::getLiveTime()
	{
		return m_nAliveTime;
	}

void BoostSession::read_handler(const boost::system::error_code& _error, size_t _readSize) {
	if (_error) {
		return;
	}
	if(_readSize > BUFFERSIZE)
	{
		std::cout << "receive msg len too large ,now buffsize is : " <<_readSize <<std::endl;
		return;
	}
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(m_cData,_readSize);
	m_pInPutQue.push_back(nodePtr);
	if(done_handler(_error))
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

void BoostSession::write_msg(const char * msg, unsigned int nMsgId,  unsigned int nLen)
{
	if(nLen + HEADSIZE > BUFFERSIZE)
	{
		std::cout <<"msglenth too long , now allow size is : " <<BUFFERSIZE<<std::endl;
		return;
	}
	char sendBuff[BUFFERSIZE] = {0};
	MsgHeadData msgHead;
	msgHead.m_nMsgId = htonl(nMsgId);
	msgHead.m_nMsgLen = htonl(nLen);
	memcpy(sendBuff,(void*)&msgHead,HEADSIZE);
	memcpy(sendBuff+HEADSIZE,msg,nLen);
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(sendBuff,nLen+HEADSIZE);
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
		boost::bind(&BoostSession::write_handler, shared_from_this(),
		boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
}
