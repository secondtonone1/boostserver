#include "StreamNode.h"

StreamNode::StreamNode(char * msg, int nLen)
{
	m_nLen = nLen;
	m_nOffSet = 0;
	m_pData = new char[m_nLen];
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
