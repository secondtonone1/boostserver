#include "Session.h"
#include <boost/bind.hpp>
#include <string.h>
#include <assert.h>
#include "../Logic/MsgDefine.h"
BoostSession::BoostSession(boost::asio::io_service& _ioService)
	:m_socket(_ioService),m_nAliveTime(boost::posix_time::second_clock::universal_time()) {
		memset(m_cData, 0, BUFFERSIZE);
		m_bPendingSend = false;
		m_bPendingRecv = false;
		m_nMsgId =0;
		m_nMsgLen = 0;
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

bool BoostSession::unserializeHead()
{
	char msgHead[HEADSIZE] = {0};
	getReadData(msgHead, HEADSIZE);
	m_nMsgId = 0;
	m_nMsgLen = 0;
	//前两个字节按位存id
	//后两个字节按位存len
	//0x 0000 0001  0001 0001 = 273 小端存储
	
	for(int i = 0; i <2; i++)
	{
		for(int j =0; j < 8; j++)
		{
			if(msgHead[i]& (0x01<<j))
				m_nMsgId+=(0x01<<j);
		}
		m_nMsgId = m_nMsgId <<8*(1-i);
	}
	if(m_nMsgId >MAXMSGID)
	{
		std::cout << "Invalid MSGID!!!"<<std::endl;
		return false;
	}
		
	for(int i = 2; i <HEADSIZE; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(msgHead[i]&(0x01<<j))
				m_nMsgLen+= (0x01<<j);
		}
		m_nMsgLen=m_nMsgLen <<8*(HEADSIZE-i-1);
	}
	if(m_nMsgLen > BUFFERSIZE || m_nMsgLen <= 0)
	{
		std::cout << "Invalid MSGLEN !!!"<<std::endl;
		return false;
	}	
	return true;
}

bool BoostSession::serializeHead(char * pData, unsigned short nMsgId, unsigned short nMsgLen)
{
	//前两个字节按位存id
	//后两个字节按位存len
	//0x 0000 0001  0001 0001 = 273
	for(unsigned int i =  0; i < 2; i++)
	{
		unsigned short nByte = (nMsgId >> 8*(1-i));
		for(int j = 0; j < 8; j++)
		{
			if(nByte & (0x01 <<j) )
				pData[i]=(pData[i]|(0x01 <<j));
		}
	}

	for(unsigned int i=2; i <HEADSIZE; i++)
	{
		unsigned short nByte = (nMsgLen >> 8*(HEADSIZE-i-1));
		for(int j=0; j<8;j++)
		{
			if(nByte & (0x01 <<j))
				pData[i]=(pData[i]|(0x01 <<j));		
		}
	}
	return true;
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
			addAvailableNode(m_pInPutQue.front());
			m_pInPutQue.pop_front();
			continue;
		}
		//新的包
		if(m_bPendingRecv == false)
		{
			//该节点接收数据小于规定包头大小
			if(readComplete(HEADSIZE) == false)
			{
				return true;
			}

			if(unserializeHead() == false)
				return false;
		
			if(readComplete(m_nMsgLen) == false)
			{
				m_bPendingRecv = true;
				return true;
			}

			//接收完全
			char strMsgData[BUFFERSIZE]={0};
			getReadData(strMsgData,m_nMsgLen);
			MsgHandlerInst::instance()->HandleMsg(m_nMsgId, strMsgData, shared_from_this());
			//std::cout <<"Receive Data : " <<strMsgData <<std::endl;
			//write_msg(strMsgData,m_msgHead.m_nMsgId,m_msgHead.m_nMsgLen);
			continue;
		}
		 //继续上次未收全的接收
		if(readComplete(m_nMsgLen) == false)
			return true;

		//接收完全
		char strMsgData[BUFFERSIZE]={0};
	    getReadData(strMsgData,m_nMsgLen);
		MsgHandlerInst::instance()->HandleMsg(m_nMsgId, strMsgData, shared_from_this());
		//std::cout <<"Receive Data : " <<strMsgData <<std::endl;
		//write_msg(strMsgData,m_msgHead.m_nMsgId,m_msgHead.m_nMsgLen);
		m_bPendingRecv = false;

	}
	return true;
}

unsigned int  BoostSession::getReadLen()
{
	int nTotal = 0;
	for( auto i = m_pInPutQue.begin(); i != m_pInPutQue.end(); i++)
	{
		nTotal += i->get()->getRemain();
	}
	return nTotal;
}

bool BoostSession::readComplete(unsigned int nLen)
{
	if(nLen == 0)
		return true;
	unsigned int nTotal = 0;
	for( auto i = m_pInPutQue.begin(); i != m_pInPutQue.end(); i++)
	{
		nTotal += i->get()->getRemain();
		if(nTotal >= nLen)
			return true;
	}
	return false;
}

bool BoostSession::getAvailableNode(streamnode_ptr & nodeptr)
{
	if(m_pAvailableQue.empty())
		return false;
	nodeptr = m_pAvailableQue.front();
	m_pAvailableQue.pop_front();
	return true;
}

bool BoostSession::addAvailableNode(const streamnode_ptr & nodeptr)
{
	if(m_pAvailableQue.size() > MAXAVAILABLE)
		return true;
	nodeptr->cleardata();
	//test 
	//std::cout << nodeptr->getself().use_count() <<std::endl; 
	m_pAvailableQue.push_back(nodeptr);
	return true;
}

unsigned int BoostSession::getReadData(char* pData, int nRead)
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
			addAvailableNode(m_pInPutQue.front());
			m_pInPutQue.pop_front();
			continue;
		}
		//节点有可读数据，且小于请求数据
		char * msgData = m_pInPutQue.front()->getMsgData();
		memcpy(pData+nCur,msgData+m_pInPutQue.front()->getOffSet(),m_pInPutQue.front()->getRemain());
		nRead-=m_pInPutQue.front()->getRemain();
		nCur+=m_pInPutQue.front()->getRemain();
		addAvailableNode(m_pInPutQue.front());
		m_pInPutQue.pop_front();
	}
	return nCur;
}

const boost::posix_time::ptime &BoostSession::getLiveTime()
{
		return m_nAliveTime;
}

void BoostSession::updateLive()
{
		m_nAliveTime = boost::posix_time::second_clock::universal_time();
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
	streamnode_ptr nodePtr;
	bool bRs = getAvailableNode(nodePtr);
	if(bRs)
	{
		nodePtr->copydata(m_cData,_readSize);
	}
	else
	{
	  nodePtr = boost::make_shared<StreamNode>(m_cData,_readSize);
	}
	
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
	//std::cout << "Send Msg Success:  "<< std::string(m_pOutPutQue.front()->getMsgData()+m_pOutPutQue.front()->getOffSet(),_writeSize)
		//<<std::endl;
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
	serializeHead(sendBuff,nMsgId,nLen);
	memcpy(sendBuff+HEADSIZE,msg,nLen);
	streamnode_ptr nodePtr;
    bool bRs = getAvailableNode(nodePtr);
	if(bRs)
	{
		nodePtr->copydata(sendBuff,nLen+HEADSIZE);
	}
	else
	{
		nodePtr  = boost::make_shared<StreamNode>(sendBuff,nLen+HEADSIZE);
	}
	
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
			addAvailableNode(m_pOutPutQue.front());
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
