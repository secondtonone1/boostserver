#include "StreamNode.h"

StreamNode::StreamNode(char * msg, int nLen)
{
	if(nLen > BUFFERSIZE)
		nLen = BUFFERSIZE;
	m_nLen = nLen;
	m_nOffSet = 0;
	m_pData = new char[BUFFERSIZE];
	memcpy(m_pData,msg,m_nLen);
}

StreamNode::~StreamNode()
{
	if(m_pData)
	{
		delete []m_pData;
		m_pData = NULL;
	}
	m_nOffSet = 0;
	m_nLen = 0;
}

std::string StreamNode::getRemainData()
{
	std::string remaindata(getMsgData()+getOffSet(),getRemain());
	return remaindata;
}

char * StreamNode::getMsgData(void)
{
	return m_pData;
}

int StreamNode::getRemain()
{
	if(m_nLen < m_nOffSet)
		m_nLen = m_nOffSet;
	return m_nLen - m_nOffSet;
}

int StreamNode::getOffSet()
{
	if(m_nOffSet>m_nLen)
		m_nOffSet = m_nLen;
	return m_nOffSet;
}

void StreamNode::resetOffset(int offsetAdd)
{
	m_nOffSet+=offsetAdd;
	if(m_nOffSet > m_nLen)
		m_nOffSet = m_nLen;
}

void StreamNode::cleardata()
{
	memset(m_pData,0,BUFFERSIZE);
	m_nLen = 0;
	m_nOffSet = 0;
}

void StreamNode::copydata(char * msg, int nLen)
{
	if(nLen > BUFFERSIZE)
		nLen = BUFFERSIZE;
	assert(m_pData);
	m_nLen = nLen;
	m_nOffSet = 0;
	memcpy(m_pData,msg,m_nLen);
}

boost::shared_ptr<StreamNode> StreamNode::getself()
{
	return shared_from_this();
}


