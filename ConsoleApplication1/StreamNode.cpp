#include "StreamNode.h"

StreamNode::StreamNode(char * msg)
{
	m_nLen = strlen(msg)+1;
	m_nOffSet = 0;
	m_pData = new char[m_nLen];
	memcpy(m_pData,msg,m_nLen-1);
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