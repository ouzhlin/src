#include "core/datautils/JODataCoder.h"
#include <stdlib.h>
#include "core/datautils/JOData.h"
#include "core/datautils/JODataUtils.h"
#include "utils/JOString.h"
#include "utils/JOLog.h"

#include "manager/JOFileMgr.h"

NS_JOFW_BEGIN


JODataCoder::JODataCoder(unsigned int opOccupyLen, unsigned int allOccupyLen)
{
	_init();
	init(opOccupyLen, allOccupyLen);
}

JODataCoder::JODataCoder(unsigned char* bytes, unsigned int len, unsigned int opOccupyLen, unsigned int allOccupyLen)
{
	_init();
	init(bytes, len, opOccupyLen, allOccupyLen);
}

void JODataCoder::_init()
{
	m_op = 0;
	m_pReadPointer = nullptr;
	m_uReadPos = 0;
	m_bBigEndian = false;	
	m_pData = new JOData(); //GET_POOL_OBJ(JOData, "JODataCoder"); //new JOData();
}

void JODataCoder::init(unsigned int opOccupyLen, unsigned int allOccupyLen)
{
	m_pData->reset();
	m_opOccupyLen = opOccupyLen;
	m_allOccupyLen = allOccupyLen;
	unsigned int oLen = opOccupyLen + allOccupyLen;

	m_pData->setLength(oLen);	

	m_pReadPointer = (unsigned char*)m_pData->bytes() + oLen;
	m_uReadPos = m_pData->length() - oLen;
}


void JODataCoder::init(unsigned char* bytes, unsigned int len, unsigned int opOccupyLen, unsigned int allOccupyLen)
{
	m_pData->reset();

	if (!bytes || len <= 0){
		LOG_ERROR("JODataCoder", "init: data should not be NULL or length should greater than 0.");
		return;
	}
	m_opOccupyLen = opOccupyLen;
	m_allOccupyLen = allOccupyLen;
	unsigned int oLen = opOccupyLen + allOccupyLen;
	m_pData->setLength(oLen);

	m_pData->appendBytes(bytes, len);
	m_pReadPointer = (unsigned char*)m_pData->bytes() + oLen;
	m_uReadPos = m_pData->length() - oLen;
}

JODataCoder::~JODataCoder()
{
	//RECOVER_POOL_OBJ(m_pData, "JOData", "JODataCoder");
	delete m_pData;
	m_pData = NULL;	
}

JOData* JODataCoder::data() const
{
	return m_pData;
}

#pragma mark JODataCoder Reader
unsigned char JODataCoder::readUByte()
{
	if(m_uReadPos >= 1)
	{
		m_uReadPos--;
		unsigned char byte = *m_pReadPointer;
		++m_pReadPointer;
		return byte;
	}
	return 0;
}

char JODataCoder::readByte()
{
	if(m_uReadPos >= 1)
	{
		m_uReadPos--;
		char byte = (char)*m_pReadPointer;
		++m_pReadPointer;
		return byte;
	}
	return 0;    
}

unsigned short JODataCoder::readUShort()
{
	if(m_uReadPos >= 2)
	{
		m_uReadPos-=2;		
		unsigned short s = JODataUtils::unsignedShortFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += 2;
		return s;
	}
	return 0; 
}

short JODataCoder::readShort()
{
	if(m_uReadPos >= 2)
	{
		m_uReadPos-=2;		
		int s = JODataUtils::shortFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += 2;
		return s;
	}
	return 0;    
}

unsigned int JODataCoder::readUInt()
{
	if(m_uReadPos >= 4)
	{
		m_uReadPos-=4;		
		unsigned int s = JODataUtils::unsignedIntFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += 4;
		return s;
	}
	return 0;      
}

int JODataCoder::readInt()
{
	if(m_uReadPos >= 4)
	{
		m_uReadPos-=4;		
		int s = JODataUtils::intFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += 4;
		return s;
	}
	return 0;    
}

unsigned long long JODataCoder::readULLong()
{
	if(m_uReadPos >= 8)
	{
		m_uReadPos-=8;		
		unsigned long long s = JODataUtils::unsignedLongLongFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += 8;
		return s;
	}
	return 0;    
}

long long JODataCoder::readLLong()
{
	if(m_uReadPos >= 8)
	{
		m_uReadPos-=8;		
		long long s = JODataUtils::longLongFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += 8;
		return s;
	}
	return 0;    
}


std::string JODataCoder::readString()
{
	unsigned short strLen = this->readUShort();
	//unsigned int strLen = this->readUInt();
	if(m_uReadPos >= strLen)
	{
		m_uReadPos-=strLen;
		
		if (m_pReadPointer != NULL)
		{
			std::string ret = std::string((const char*)m_pReadPointer, strLen);
			/*
			char* pStr = (char*)malloc(strLen+1);
			if (pStr != NULL)
			{
				pStr[strLen] = '\0';
				if (strLen > 0)
				{
					memcpy(pStr, m_pReadPointer, strLen);
				}
				ret = pStr;				
				free(pStr);
			}
			*/
			m_pReadPointer += strLen;
			return ret;
		}
	}
	return "";
}


float JODataCoder::readFloat()
{
	if(m_uReadPos >= sizeof(float))
	{
		m_uReadPos-=sizeof(float);		
		float f = JODataUtils::floatFormData(m_pReadPointer, m_bBigEndian);
		m_pReadPointer += sizeof(float);
		return f;
	}
	return 0.0;   
	
	//f=((int)(f*100+0.5f));//100.0;
	//f=f/100.0f;	
}

#pragma mark JODataCoder Writer
void JODataCoder::writeUByte(unsigned char byte)
{
	m_pData->appendBytes(&byte, 1);	
	m_uReadPos += 1;
}

void JODataCoder::writeByte(char byte)
{
	m_pData->appendBytes((unsigned char*)&byte, 1);
	m_uReadPos += 1;
}

void JODataCoder::writeUShort(unsigned short s)
{
	unsigned char buff[2];
	JODataUtils::dataFormUnsignedShort(s, buff, m_bBigEndian);
	m_pData->appendBytes(buff, 2);
	m_uReadPos += 2;
}

void JODataCoder::writeShort(short s)
{
	unsigned char buff[2];
	JODataUtils::dataFormShort(s, buff, m_bBigEndian);
	m_pData->appendBytes(buff, 2);
	m_uReadPos += 2;
}

void JODataCoder::writeUInt(unsigned int i)
{
	unsigned char buff[4];
	JODataUtils::dataFormUnsignedInt(i, buff, m_bBigEndian);
	m_pData->appendBytes(buff, 4);
	m_uReadPos += 4;
}

void JODataCoder::writeInt(int i)
{
	unsigned char buff[4];
	JODataUtils::dataFormInt(i, buff, m_bBigEndian);
	m_pData->appendBytes(buff, 4);
	m_uReadPos += 4;
}

void JODataCoder::writeULLong(unsigned long long l)
{
	unsigned char buff[8];
	JODataUtils::dataFormUnsignedLongLong(l, buff, m_bBigEndian);
	m_pData->appendBytes(buff, 8);
	m_uReadPos += 8;
}

void JODataCoder::writeLLong(long long l)
{
	unsigned char buff[8];
	JODataUtils::dataFormLongLong(l, buff, m_bBigEndian);
	m_pData->appendBytes(buff, 8);
	m_uReadPos += 8;
}

void JODataCoder::writeString(const std::string& str)
{
	static unsigned short s_maxUS = -1;

	unsigned int byteSize = str.length();
	if (byteSize > 0){
		if (byteSize > s_maxUS){
			LOG_WARN("JODataCoder", "string too [%s]", str.c_str());
			LOG_WARN("JODataCoder", "string too long over unsigned short %d", s_maxUS);
			byteSize = s_maxUS;
		}

		writeShort(byteSize);
		m_pData->appendBytes((const unsigned char*)str.c_str(), byteSize);
		m_uReadPos += byteSize;
	}
	else{
		writeShort(0);
		//LOG_WARN("JODataCoder", "is empty string !!!");
	}
	/*
	unsigned char* c = new unsigned char[byteSize+2];
	//JODataUtils::dataFormUnsignedInt(byteSize, c, m_bBigEndian);
	JODataUtils::dataFormUnsignedShort(byteSize, c, m_bBigEndian);
	memcpy(c+2, str.c_str(), byteSize);
	m_pData->appendBytes(c, byteSize+2);
	JO_SAFE_DELETE_ARRAY(c);
	*/
	
	/*
	memcpy(c, str.c_str(), byteSize);
	this->writeUnsignedShort(byteSize);
	m_pData->appendBytes(c, byteSize);
	*/
	
}


void JODataCoder::writeFloat( float f )
{	
	unsigned char buff[JO_SIZEOF_FLOAT];
	JODataUtils::dataFormfloat(f, buff, m_bBigEndian);
	
	m_pData->appendBytes(buff, JO_SIZEOF_FLOAT);
	m_uReadPos += JO_SIZEOF_FLOAT;
	/*
	unsigned char* buf = new unsigned char[sizeof(float)];
	JODataUtils::dataFormfloat(f, buf, m_bBigEndian);
	m_pData->appendBytes(buf, sizeof(float));
	delete buf;
	*/
}


JOData* JODataCoder::netPack()
{
	unsigned char buff[2];
	if (m_allOccupyLen>0)
	{//-2为去掉长度的占位
		JODataUtils::dataFormUnsignedShort(m_pData->length()-2, buff, m_bBigEndian);
		m_pData->replaceBytesInRange(0, 2, buff, 2);
	}
	if (m_opOccupyLen>0)
	{
		JODataUtils::dataFormShort(m_op, buff, m_bBigEndian);
		m_pData->replaceBytesInRange(2, 4, buff, 2);
	}
	else{
		LOG_WARN("JODataCoder", "no occupy data !!!!!");
	}
	return m_pData;
}

std::string JODataCoder::description()
{
	unsigned int oLen = m_opOccupyLen+m_allOccupyLen;
	if (m_op){
		return JOString::formatString("op=%d data=%s", m_op, JOString::data2String(m_pData->bytes() + oLen, m_pData->length() - oLen).c_str());
	}
	return JOString::data2String(m_pData->bytes() + oLen, m_pData->length() - oLen);
}

unsigned int JODataCoder::length()
{
	unsigned int oLen = m_opOccupyLen + m_allOccupyLen;
	return m_pData->length() - oLen;
}

void JODataCoder::seek( unsigned int pos )
{
	unsigned int oLen = m_opOccupyLen + m_allOccupyLen;
	
	if (m_pData->length() - oLen<=pos)return;
	//m_pReadPointer = (unsigned char*)(m_pData->bytes()+pos);
	//m_uReadPos = m_pData->length()-pos;
	m_pReadPointer = (unsigned char*)m_pData->bytes() + oLen + pos;
	m_uReadPos = m_pData->length() - oLen - pos;
}

char JODataCoder::getByte()
{
    return readByte();
}
short JODataCoder::getShort()
{
    return readShort();
}

int JODataCoder::getInt()
{
    return readInt();
}
bool JODataCoder::getBool()
{
    char value = 0;
    value = readByte();
    if (0 < value)
    {
        return true;
    }
    return false;
}
std::string JODataCoder::getLong()
{
    return JOString::formatString("%lld",readLLong());
}
std::string JODataCoder::getString()
{
    return readString();
}

std::string JODataCoder::getFloat()
{
    return JOString::formatString("%f",readFloat());
}

void JODataCoder::putByte(char c)
{
    writeByte(c);
}
void JODataCoder::putShort(short s)
{
    writeShort(s);
}
void JODataCoder::putInt(int i)
{
    writeInt(i);
}
void JODataCoder::putBool(bool b)
{
    writeByte(b);
}
void JODataCoder::putLong(std::string ll)
{
    long long val = JOString::toLongLong(ll.c_str());
    writeLLong(val);
}
void JODataCoder::putString(const std::string& str)
{
    writeString(str);
}
void JODataCoder::putFloat(float f)
{
    writeFloat(f);
}

bool JODataCoder::wirte2File(const std::string& filename)
{
	return JOFileMgr::wirteFile(filename.c_str(), m_pData->bytes(), m_pData->length());
}

bool JODataCoder::readFile(const std::string& filename)
{
	JOData* fdata = JOFileMgr::Instance()->getFileData(filename);
	if (fdata){
		m_opOccupyLen = 0;
		m_allOccupyLen = 0;
		m_pData->setData(fdata);
		m_pReadPointer = (unsigned char*)m_pData->bytes();
		m_uReadPos = m_pData->length();
		return true;
	}
	return false;
}

NS_JOFW_END