#ifndef __BOOST_SESSION_H__
#define __BOOST_SESSION_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <deque>
#include "StreamNode.h"

enum TCPSTATE{
	TCPSUCCESS = 0,//TCP处理完全
	TCPHEADLESS,  //TCP头部未收全
	TCPHEADERROR, //TCP头部错误
	TCPDATALESS, //TCP数据域未收全
};

enum WEBSTATE{
	WEBHANDSHAKESUCCESS = 0, // 握手成功
	WEBHANDSHAKELESS , //握手信息未收全
	WEBHANDSHAKEFAIL, //握手失败
	WEBSTEPSUCCESS,//分步处理成功
	WEBSOCKETNONE, //websocket 数据为空
	WEBSOCKETFINISHBIT, //websocket finishbit收全
	WEBSOCKETLENANALY,//websocket len 分析完
	WEBSOCKETLEN,  //websocket len 字段接收完全
	WEBMASKLESS,//websocket mask 未接受全
	WEBSOCKETDATALESS,//websocket 数据粘包未收全
	WEBSOCKETSUCCESS, //websocket 处理成功
	WEBSOCKETCLOSE,//websocket close
};


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
	void write_webmsg(const char* msg, unsigned int nLen);
	void responclient(char respdata[],int datalen);
private:
	// 完成数据传输后触发的收尾工作
	bool done_handler(const boost::system::error_code& _error);
	// 读取成功后触发的函数
	void read_handler(const boost::system::error_code& _error, size_t _readSize);
	// 写入完成后触发的函数
	void write_handler(const boost::system::error_code& _error, size_t _writeSize);
	void async_send();
	unsigned int  getReadLen();
	bool readComplete(unsigned int nLen);
	//std::string  getReadData(int nDataLen = 0);
	unsigned int getReadData(char* pData, int nRead);
	bool getAvailableNode(streamnode_ptr & nodeptr);
	bool addAvailableNode(const streamnode_ptr & nodeptr);
	bool unserializeHead();
	bool serializeHead(char * pData, unsigned short nMsgId, unsigned short nMsgLen);
	int  handleTcp();
	int  handleWeb();
	int  handleHandShake();
	//确定消息类型
    void confirmType();
	//处理第一个字节
	int  handleFirstByte(char * msgData, int & index, int & nRemain);
	//处理第二个字节
	int  handleSecondByte(char * msgData, int & index, int & nRemain);
	//处理长度数据
	int handleLenByte(char * msgData, int & index, int & nRemain);
	//处理maskbit
	int handleMaskBit(char * msgData, int & index, int & nRemain);
	//处理数据
	int handleWebData(char * msgData, int & index, int & nRemain);
	void clearWebFlags();
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
	//心跳时间
	boost::posix_time::ptime  m_nAliveTime;
	//消息id
	unsigned short m_nMsgId;
	//消息长度
	unsigned short m_nMsgLen;
	//是否是websocket通信
	bool m_bWebSocket;
	//是否确认通信类型tcp or websocket
	bool m_bTypeConfirm;
	//是否已经进行握手
	bool m_bWebHandShake;
	
	int    m_nFinishbit;//0有后续包，1对端发送完全
	char m_cWebData[BUFFERSIZE];
	int    m_nWebState;
	int    m_nWebLen;
	int    m_nMaskBit;
	char m_cMaskKey[4];
	int    m_nLenPend;  //长度处理进度
	int    m_nMaskPend; //mask 处理进度
	int    m_nWebDataPend; //webdata 处理进度
	char m_cLenArray[BUFFERSIZE]; 
};

typedef	boost::shared_ptr<BoostSession>	session_ptr;
typedef boost::weak_ptr<BoostSession> weak_session_ptr;

#endif //__BOOST_SESSION_H__