
//#include <stdio.h>
#include "core/datautils/JOData.h"
#include "utils/JOString.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

#define GROWTH_RANGE 128
const char kBlockSizes = 6;
const static unsigned int block_sizes_[kBlockSizes] =
{
	8,16,32,64,96,128
};

static int getBlockSize(unsigned int tempSize)
{
	for (unsigned int i = 0; i < kBlockSizes; ++i){
		if (tempSize < block_sizes_[i])	{
			return block_sizes_[i];			
		}
	}
	return tempSize + tempSize*0.5;	
}

JOData::JOData():m_nTotalLength(0), m_nLength(0), m_pData(nullptr)
{
	_initData();	
}

JOData::JOData(unsigned char *pBytes, unsigned int size) : m_nTotalLength(0), m_nLength(0), m_pData(nullptr)
{
	/*
    if (size < 1)
    {
        LOG_WARN("JOData", "size < 1");
        m_pData = new unsigned char[1];
        memset(m_pData,0,1);
        m_nLength = 1;
        return;
    }
	m_pData = new unsigned char[size];
	memcpy(m_pData, pBytes, m_nLength);	
	*/
	//////////////////////////////////////////////////////////////////////////
	_initData();
	setData(pBytes, size);
}

JOData::JOData(JOData* data) :m_nTotalLength(0), m_nLength(0), m_pData(nullptr)
{
	_initData();
	setData(data);
}

JOData::~JOData(void)
{	
    JO_SAFE_DELETE_ARRAY(m_pData);
}

void JOData::setData(JOData* data)
{	
	unsigned int len = data->length();
	if (len > m_nTotalLength)
	{
		m_nTotalLength = getBlockSize(len);
		JO_SAFE_DELETE_ARRAY(m_pData);
		m_pData = new unsigned char[m_nTotalLength];
	}

	if (len < 1)
	{
		m_nLength = 0;
	}
	else
	{
		m_nLength = len;
		memcpy(m_pData, data->bytes(), len);
	}
}

void JOData::setData(unsigned char *pBytes, unsigned int size)
{
	if (size > m_nTotalLength)
	{
		m_nTotalLength = getBlockSize(size);
		JO_SAFE_DELETE_ARRAY(m_pData);
		m_pData = new unsigned char[m_nTotalLength];
	}

	if (size < 1)
	{
		m_nLength = 0;
	}
	else
	{
		m_nLength = size;
		memcpy(m_pData, pBytes, size);
	}
}

void JOData::reset()
{
	m_nLength = 0;
	memset(m_pData, 0, m_nTotalLength);
}

void JOData::_initData()
{
	m_nTotalLength = block_sizes_[0];
	m_nLength = 0;
	m_pData = new unsigned char[m_nTotalLength];
}

const unsigned char* JOData::bytes()
{
    return m_pData;
}

// Adjusting Capacity
void JOData::increaseLength(unsigned int extraLen)
{
	setLength(m_nTotalLength+extraLen);
}

void JOData::setLength(unsigned int length)
{
	if (length == m_nTotalLength)
	{
		return;
	}

	if (length <= 0)
	{
		JO_SAFE_DELETE_ARRAY(m_pData);
		m_pData = NULL;
		m_nLength = 0;
		m_nTotalLength = 0;
		return;
	}

	m_nTotalLength = length;
	unsigned char* newBuffer = new unsigned char[m_nTotalLength];
	if (length < m_nLength)
	{
		memcpy(newBuffer,m_pData, length);
		m_nLength = length;
	}
	else 
	{
		memcpy(newBuffer,m_pData, m_nLength);
	}
	JO_SAFE_DELETE_ARRAY(m_pData);
	m_pData = newBuffer;
}

// Adding Data
void JOData::appendBytes(const unsigned char *pBytes, unsigned int size)
{
	if (size <= 0)
	{		
		LOG_ERROR("JOData","append size should not <= 0");		
		return;
	}	
	if (m_nLength + size > m_nTotalLength)
	{
		unsigned int tempSize = (m_nLength + size) - m_nTotalLength;
		tempSize = getBlockSize(tempSize);
		increaseLength(tempSize);
	}
	if (pBytes)
		memcpy(this->m_pData + m_nLength, pBytes, size);
	else
		memcpy(this->m_pData + m_nLength, 0, size);
	m_nLength = m_nLength + size;
}

void JOData::appendData(JOData* data)
{
	appendBytes(data->bytes(),data->length());
}

void JOData::insertBytes(const unsigned char* pBytes, unsigned int size, unsigned int begin /*= 0*/)
{
    if (pBytes == NULL || size <= 0)
    {
        LOG_ERROR("JOData","insert bytes should not be NULL");
        return;
    }

	if (m_nLength + size > m_nTotalLength)
	{
		unsigned int tempSize = (m_nLength + size) - m_nTotalLength;
		tempSize = getBlockSize(tempSize);
		m_nTotalLength = m_nTotalLength + tempSize;
		unsigned char* newBuffer = new unsigned char[m_nTotalLength];
		if (begin>0)
			memcpy(newBuffer, m_pData, begin);
		memcpy(newBuffer + begin, pBytes, size);
		memcpy(newBuffer + begin + size, m_pData + begin, m_nLength - begin);
		JO_SAFE_DELETE_ARRAY(m_pData);
		m_pData = newBuffer;
	}
	else
	{
		memmove(m_pData + begin + size, m_pData + begin, m_nLength - begin);
		memmove(m_pData + begin, pBytes, size);
	}
	
    m_nLength = m_nLength+size;
}
// Modifying Data
void JOData::replaceBytesInRange(unsigned int begin, unsigned int end, void* pBytes, unsigned int size)
{
	
	if (begin < 0 || end <= 0 || begin - end >= 0 || begin-m_nLength >= 0)
	{
		LOG_ERROR("JOData"," replaceBytesInRange invalid range!");
		return;
	}

	unsigned int e = end;
	if (m_nLength < e)
	{
		e = m_nLength;
	}
	
	if (pBytes == NULL && size == 0)
	{
		memcpy(this->m_pData+begin,0,e-begin);
	}
	else
	{
		unsigned int s = e-begin;
		if (s > size)
		{
			s = size;
		}

		memcpy(this->m_pData+begin,pBytes,s);
	}
}

void JOData::resetBytes(unsigned int begin, unsigned int end)
{
	if (end >= m_nLength)
	{
		this->replaceBytesInRange(begin, m_nLength, NULL, 0);
	}
	else
	{
		this->replaceBytesInRange(begin,end,NULL,0);
	}
}

std::string JOData::description()
{
	return JOString::data2String((const char*)m_pData, m_nLength);
	/*
    const unsigned int desSize = m_nLength*2+m_nLength/4+2;
    char* des = new char[desSize];
    memset(des, 0, desSize);
    for (unsigned int i = 0; i < m_nLength; i++) {
        sprintf(des, "%s%.2x",des,m_pData[i]);
        if ((i+1)%4 == 0 && i < m_nLength-1) {
            sprintf(des, "%s ",des);
        }
    }
    std::string desc = "<";
	desc.append(des);
	desc.append(">");
    delete des;
    return desc;
	*/
}

void JOData::getBytes(void* buffer, unsigned int len)
{
	if (len > m_nLength)
	{
		memcpy(buffer, m_pData, m_nLength);
	}
	else
	{
		memcpy(buffer,m_pData,len);
	}
}


bool JOData::isEqualToData(JOData* data)
{
	if (length() != data->length())
	{
		return false;
	}
	
	for (unsigned int i = 0; i < m_nLength; i++)
	{
		if (m_pData[i] != data->m_pData[i])
		{
			return false;
		}
	}
	return true;
}

NS_JOFW_END