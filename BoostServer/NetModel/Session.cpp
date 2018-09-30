#include "Session.h"
#include <boost/bind.hpp>
#include <string.h>
#include <assert.h>
#include "../Logic/MsgDefine.h"
#include<boost/uuid/detail/sha1.hpp>
#include "Base64.h"
BoostSession::BoostSession(boost::asio::io_service& _ioService)
	:m_socket(_ioService),m_nAliveTime(boost::posix_time::second_clock::universal_time()) {
		memset(m_cData, 0, BUFFERSIZE);
		m_bPendingSend = false;
		m_bPendingRecv = false;
		m_nMsgId =0;
		m_nMsgLen = 0;
		m_bWebSocket = false;
		m_bTypeConfirm = false;
		m_bWebHandShake = false;
		m_nWebDataRemain = false;
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
	//first two bytes save as bit for id
	//last two bytes save as bit for len
	//0x 0000 0001  0001 0001 = 273 Little Endian
	
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
	//first two bytes save as bit for id
	//last two bytes save as bit for len
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

int  BoostSession::handleTcp()
{
	//new package
	if(m_bPendingRecv == false)
	{
		//head data didn't receive completely
		if(readComplete(HEADSIZE) == false)
		{
			return TCPHEADLESS;
		}
		//unserializeHead failed
		if(unserializeHead() == false)
			return TCPHEADERROR;
		//MSG data did not receive completely, msg length is less than m_nMsgLen
		if(readComplete(m_nMsgLen) == false)
		{
			m_bPendingRecv = true;
			return TCPDATALESS;
		}

		//MSG data  received completely, msg len is m_nMsgLen
		char strMsgData[BUFFERSIZE]={0};
		getReadData(strMsgData,m_nMsgLen);
		MsgHandlerInst::instance()->HandleMsg(m_nMsgId, strMsgData, shared_from_this());
		return TCPSUCCESS;
	}
	//m_bPendingRecv is true, It means the last head data received completely, MSG data did not receive completely
	//The news is still incomplete
	if(readComplete(m_nMsgLen) == false)
		return TCPDATALESS;

	//Message receive complete 
	char strMsgData[BUFFERSIZE]={0};
	getReadData(strMsgData,m_nMsgLen);
	MsgHandlerInst::instance()->HandleMsg(m_nMsgId, strMsgData, shared_from_this());
	m_bPendingRecv = false;
	return TCPSUCCESS;
}

//The handshake format requested by the client
//GET / HTTP/1.1
//Host: 192.168.1.172:8898
//Connection: Upgrade
//Pragma: no-cache
//Cache-Control: no-cache
//Upgrade: websocket
//Origin: http://coolaf.com\r\nSec-WebSocket-Version: 13\r\nUser-Agent: Mozilla/...

 void BoostSession::confirmType()
 {
	 if(m_bTypeConfirm)
		 return;
	 if(m_pInPutQue.empty())
		 return;
	std::string & typeStr=m_pInPutQue.front()->getRemainData();
	std::string::size_type pos = typeStr.find("GET / HTTP/1.1");
	if (pos != std::string::npos)
	{
		m_bWebSocket=true;
		m_bTypeConfirm = true;
		return;
	}
	pos = typeStr.find("GET /chat HTTP/1.1");
	if (pos != std::string::npos)
	{
		m_bWebSocket=true;
		m_bTypeConfirm = true;
		return;
	}
	m_bWebSocket=false;
	 m_bTypeConfirm = true;
	 return;
 }

int  BoostSession::handleWeb()
{
	if(m_pInPutQue.empty())
	{
		return WEBSOCKETDATALESS;
	}
	//unpack webhead package
	int index=0;		
	int nRemain=m_pInPutQue.front()->getRemain();
	if(nRemain==0)
		return WEBSOCKETCLOSE;
	int nOffset= m_pInPutQue.front()->getOffSet();
	index+=nOffset;
	char * msgData= m_pInPutQue.front()->getMsgData();
	char firstByte=msgData[index];
	//first byte
	int finishbit=(firstByte&0x80)>>7;
	int  opcode=(int)(firstByte&0x0F);
	if(opcode==8)
		return WEBSOCKETCLOSE;
	//second byte
	index++;
	char secondByte=msgData[index];
	int maskbit = (secondByte&0x80)>>7;
	int datalen = (int)(msgData[index] & 0x7F);
	if(datalen==126)
	{
		char extended[2] = {0};
		extended[0] = msgData[++index];
		extended[1] = msgData[++index];
		int shift = 0;
		datalen = 0;
		for (int i = 2- 1; i >= 0; i--) {
			datalen = datalen + ((extended[i] & 0xFF) << shift);
			shift += 8;
		}
	}
	else if(datalen==127)
	{

	}
	return WEBSOCKETSUCCESS;
}

//he handshake format,The server replies to t
//HTTP/1.1 101 Switching Protocols
//Upgrade: websocket
//Connection: Upgrade
//Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
//Sec-WebSocket-Protocol: chat

int  BoostSession::handleHandShake()
{
	if(m_pInPutQue.empty())
	{
		return WEBHANDSHAKELESS;
	}
	m_bWebHandShake = true;
	char shakedata[BUFFERSIZE]={0};
	getReadData(shakedata,BUFFERSIZE);
	std::string strShakedata(shakedata);
	std::string::size_type  socketkeyindex = strShakedata.find("Sec-WebSocket-Key");
	if (socketkeyindex == std::string::npos)
		return WEBHANDSHAKEFAIL;
	std::string keyStr = strShakedata.substr(socketkeyindex + 19, 24);
	//Construct the server handshake resp message
	char request[BUFFERSIZE]={0};
	strcat(request, "HTTP/1.1 101 Switching Protocols\r\n");
	strcat(request, "Connection: Upgrade\r\n");
	strcat(request, "Sec-WebSocket-Accept: ");
	keyStr += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	boost::uuids::detail::sha1 sha;
	unsigned int message_digest[5];
	sha.reset();
	sha.process_bytes(keyStr.c_str(), keyStr.length());
	sha.get_digest(message_digest);
	
	for(int i=0; i <5; i++){
		std::cout<< std::hex << message_digest[i]; 
	}
	
	for (int i = 0; i < 5; i++) {
		message_digest[i] = htonl(message_digest[i]);
	}
	cout << endl;	
	std::string server_key = Base64HandlerInst::instance()->base64_encode( reinterpret_cast<const unsigned char*>(message_digest),20);
	server_key += "\r\n";
	strcat(request, server_key.c_str());
	strcat(request, "Upgrade: websocket\r\n\r\n");
	cout << "after encode base64: "<< server_key <<endl;
	write_webmsg(request,strlen(request));

	return WEBHANDSHAKESUCCESS;
}

// Process after receiving the message
bool BoostSession::done_handler(const boost::system::error_code& _error) {
	if (_error) {
		return false;
	}
	while(!m_pInPutQue.empty())
	{
		int nRemain = m_pInPutQue.front()->getRemain();
		//The node data is parsed and processed completely
		if(nRemain == 0)
		{
			addAvailableNode(m_pInPutQue.front());
			m_pInPutQue.pop_front();
			continue;
		}
		//Message type not determined, websocket or tcp
		if(m_bTypeConfirm==false)
		{
			confirmType();	
		}
		//handle tcp msg
		if(m_bWebSocket == false)
		{
			int  tcpState = handleTcp();
			if(tcpState == TCPSUCCESS )
				continue;
			if(tcpState == TCPHEADLESS || tcpState == TCPDATALESS)
				return true;
			if(tcpState == TCPHEADERROR)
				return false;
		}
		else 
		{
			//Process the web handshake request
			if(m_bWebHandShake == false)
			{
				int handshake=handleHandShake();
				if(WEBHANDSHAKEFAIL==handshake)
					return false;
				continue;
			}
			//Handle websocket communication
			{
				handleWeb();
			}
		}
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
		//the node readable data is more than nRead
		if(m_pInPutQue.front()->getRemain() >= nRead)
		{
			char * msgData =m_pInPutQue.front()->getMsgData();
			memcpy(pData+nCur,msgData+m_pInPutQue.front()->getOffSet(),nRead);
			nCur+=nRead;
			m_pInPutQue.front()->resetOffset(nRead);
			return nCur;
		}
		//the node readable data is empty
		if(m_pInPutQue.front()->getRemain() == 0)
		{
			addAvailableNode(m_pInPutQue.front());
			m_pInPutQue.pop_front();
			continue;
		}
		//the node readable data is less than nRead
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

void BoostSession::write_webmsg(const char* msg, unsigned int nLen)
{
	if(nLen > BUFFERSIZE)
	{
		std::cout << "msglenth too long , now allow size is : " <<BUFFERSIZE<<std::endl;
		return;
	}

	char sendBuff[BUFFERSIZE] = {0};
	memcpy(sendBuff,msg,nLen);
	streamnode_ptr nodePtr;
	bool bRs = getAvailableNode(nodePtr);
	if(bRs)
	{
		nodePtr->copydata(sendBuff,nLen);
	}
	else
	{
		nodePtr  = boost::make_shared<StreamNode>(sendBuff,nLen);
	}

	m_pOutPutQue.push_back(nodePtr);
	if(!m_bPendingSend)
	{
		m_bPendingSend = true;
		async_send();
	}
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
		//find the node , which data is not empty
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
