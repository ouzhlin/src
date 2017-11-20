/********************************************************************
CREATED: 25/1/2014   11:56
FILE: 	 JOStrings.cpp
AUTHOR:  James Ou 
*********************************************************************/

#include "utils/JOString.h"

#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <ostream>
#include <stdarg.h>
#include <stdlib.h>

NS_JOFW_BEGIN


//http://dl.google.com/android/ndk/android-ndk-r9c-darwin-x86_64.tar.bz2
/**************************************************************************** 
函数名称: str_to_hex
函数功能: 字符串转换为十六进制
输入参数: string 字符串 cbuf 十六进制 len 字符串的长度。
输出参数: 无
*****************************************************************************/   
int JOString::str_to_hex(char *string, unsigned char *cbuf, int len)  
{  
	unsigned char high, low;  
	unsigned int idx, ii=0;
	for (idx=0; idx<len; idx+=2)   
	{  
		high = string[idx];  
		low = string[idx+1];  

		if(high>='0' && high<='9')  
			high = high-'0';  
		else if(high>='A' && high<='F')  
			high = high - 'A' + 10;  
		else if(high>='a' && high<='f')  
			high = high - 'a' + 10;  
		else  
			return -1;  

		if(low>='0' && low<='9')  
			low = low-'0';  
		else if(low>='A' && low<='F')  
			low = low - 'A' + 10;  
		else if(low>='a' && low<='f')  
			low = low - 'a' + 10;  
		else  
			return -1;  

		cbuf[ii++] = high<<4 | low;  
	}  
	return 0;  
}  

/**************************************************************************** 
函数名称: hex_to_str
函数功能: 十六进制转字符串
输入参数: ptr 字符串 buf 十六进制 len 十六进制字符串的长度。
输出参数: 无
*****************************************************************************/   
void JOString::hex_to_str(char *ptr,unsigned char *buf,int len)
{  
	for(unsigned int i = 0; i < len; i++)  
	{  
		sprintf(ptr, "%02x",buf[i]);  
		ptr += 2;  
	}  
}  

//-----------------------------------------------------------------------
void JOString::trim(string& str, bool left, bool right)
{
    /*
    size_t lspaces, rspaces, len = length(), i;

    lspaces = rspaces = 0;

    if( left )
    {
        // Find spaces / tabs on the left
        for( i = 0;
            i < len && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
            ++lspaces, ++i );
    }
        
    if( right && lspaces < len )
    {
        // Find spaces / tabs on the right
        for( i = len - 1;
            i >= 0 && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
            rspaces++, i-- );
    }

    *this = substr(lspaces, len-lspaces-rspaces);
    */
    static const string delims = " \t\r";
    if(right)
        str.erase(str.find_last_not_of(delims)+1); // trim right
    if(left)
        str.erase(0, str.find_first_not_of(delims)); // trim left
}

//-----------------------------------------------------------------------
vector<string> JOString::split( const string& str, const string& delims, unsigned int maxSplits)
{
    vector<string> ret;
    // Pre-allocate some space for performance
    ret.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

    unsigned int numSplits = 0;

    // Use STL methods 
    size_t start, pos;
    start = 0;
    do 
    {
        pos = str.find_first_of(delims, start);
        if (pos == start)
        {
            // Do nothing
            start = pos + 1;
        }
        else if (pos == string::npos || (maxSplits && numSplits == maxSplits))
        {
            // Copy the rest of the string
            ret.push_back( str.substr(start) );
            break;
        }
        else
        {
            // Copy up to delimiter
            ret.push_back( str.substr(start, pos - start) );
            start = pos + 1;
        }
        // parse up to next real data
        start = str.find_first_not_of(delims, start);
        ++numSplits;

    } while (pos != string::npos);



    return ret;
}


string JOString::concat(vector<string> vs, const string& delims /*= ""*/)
{
	string ret;
	vector<string>::iterator itr = vs.begin();
	if (itr != vs.end())
	{
		ret.append((*itr).begin(), (*itr).end());
		++itr;
	}
	while (itr != vs.end())
	{
		ret.append(delims.begin(), delims.end());
		ret.append((*itr).begin(), (*itr).end());
		++itr;
	}
	return ret;
}


bool JOString::isContains(const string& src, const string& sub)
{
	string::size_type pos = src.find(sub, 0);
	if (pos != string::npos){
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------
void JOString::toLowerCase(string& str)
{
    transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);
}

//-----------------------------------------------------------------------
void JOString::toUpperCase(string& str) 
{
    transform(
        str.begin(),
        str.end(),
        str.begin(),
		(int (*)(int))toupper);
}
//-----------------------------------------------------------------------
bool JOString::startsWith(const string& str, const string& pattern, bool lowerCase)
{
    size_t thisLen = str.length();
    size_t patternLen = pattern.length();
    if (thisLen < patternLen || patternLen == 0)
        return false;

    string startOfThis = str.substr(0, patternLen);
    if (lowerCase)
        JOString::toLowerCase(startOfThis);

    return (startOfThis == pattern);
}
//-----------------------------------------------------------------------
bool JOString::endsWith(const string& str, const string& pattern, bool lowerCase)
{
    size_t thisLen = str.length();
    size_t patternLen = pattern.length();
    if (thisLen < patternLen || patternLen == 0)
        return false;

    string endOfThis = str.substr(thisLen - patternLen, patternLen);
    if (lowerCase)
        JOString::toLowerCase(endOfThis);

    return (endOfThis == pattern);
}

bool JOString::isEqual( const char* str1, const char* str2 )
{
	if (str1 == NULL || str2 == NULL)
		return false;
	return strcmp(str1, str2) == 0;
}

const unsigned int BufMaxSize = 1024 * 100;
static char szBuf[BufMaxSize];
string JOString::formatString( const char* fmt , ... )
{
	
	va_list ap;
	va_start(ap, fmt);
	//vsprintf(szBuf,  fmt, ap);
	vsnprintf(szBuf, BufMaxSize, fmt, ap);
	va_end(ap);
	szBuf[BufMaxSize - 1] = '\0';
	return string(szBuf);
	/*
	va_list ap;
	va_start(ap, fmt);
	vector<char> buf(1024);
	
	while (vsnprintf_s(&buf[0], buf.size(), buf.size(), fmt, ap) == -1)
	{
		buf.resize(buf.size() * 2);
	}
	va_end(ap);

	结尾不是NULL，则截断
	if (static_cast<int>(buf.size()))
	{
		buf[buf.size() - 1] = '\0';
	}

	return string(&buf[0]);// shared_ptr<string>(new string(&buf[0]));
	*/	
}


int JOString::getUTF8CharacterLength( const char* p, int len )
{
	unsigned int ret = 0;

	if (len >= 6 && (*p & 0xfe) == 0xfc 
		&& (*(p+1) & 0xc0) == 0x80
		&& (*(p+2) & 0xc0) == 0x80
		&& (*(p+3) & 0xc0) == 0x80
		&& (*(p+4) & 0xc0) == 0x80
		&& (*(p+5) & 0xc0) == 0x80) 
	{
		ret = 6;
	}
	else if (len >= 5 && (*p & 0xfc) == 0xf8 
		&& (*(p+1) & 0xc0) == 0x80
		&& (*(p+2) & 0xc0) == 0x80
		&& (*(p+3) & 0xc0) == 0x80
		&& (*(p+4) & 0xc0) == 0x80) 
	{
		ret = 5;
	}
	else if(len >= 4 && (*p & 0xf8) == 0xf0 
		&& (*(p+1) & 0xc0) == 0x80
		&& (*(p+2) & 0xc0) == 0x80
		&& (*(p+3) & 0xc0) == 0x80) 
	{
		ret = 4;
	}
	else if (len >= 3 && (*p & 0xf0) == 0xe0 
		&& (*(p+1) & 0xc0) == 0x80
		&& (*(p+2) & 0xc0) == 0x80)
	{
		ret = 3;
	}
	else if( len >= 2 && (*p & 0xe0) == 0xc0 
		&& (*(p+1) & 0xc0) == 0x80) {
		ret = 2;
	}
	else if (*p < 0x80)
	{
		/*
		英文或者数字
		*/
		ret = 1;
	}

	return ret;
}

vector<int> JOString::getUTF8StringSplitInfo( const char* pText )
{
	vector<int> splitInfo;
	if (!pText)
		return splitInfo;

	splitInfo.clear();

	const char* pStr = pText;
	int len = (int)strlen(pText);
	unsigned int strLen  = 0;
	while (*pStr != '\0')
	{
		strLen = getUTF8CharacterLength(pStr, len);
		if (strLen == 0)
			break;

		if (splitInfo.empty())
			splitInfo.push_back(strLen);
		else
			splitInfo.push_back(splitInfo.back()+strLen);

		len -= strLen;
		pStr+=strLen;
	}

	return splitInfo;
}


string JOString::convertBreakLine( const char* pszText )
{
	if (pszText)
	{
		return replaceAll(pszText, "\\n", "\n");
	}

	return string(pszText);
}

string JOString::replaceAll(const char* pszText, const char* pszSrc, const char* pszDest, bool bAll/* = true*/)
{
	string ret;

	if (pszText && pszSrc && pszDest)
	{
		ret = pszText;
		string::size_type pos = 0;
		string::size_type srcLen = strlen(pszSrc);
		string::size_type desLen = strlen(pszDest);
		pos = ret.find(pszSrc, pos); 
		if (bAll){
			while ((pos != string::npos)){
				ret = ret.replace(pos, srcLen, pszDest);
				pos = ret.find(pszSrc, (pos + desLen));
			}
		}
		else{
			if ((pos != string::npos)){
				ret = ret.replace(pos, srcLen, pszDest);				
			}
		}
		
	}

	return ret;
}

int JOString::stringCount( const char* pTex )
{
	return getUTF8StringSplitInfo(pTex).size();
}


unsigned char JOString::convertFromHex( string hex )
{
	int value = 0;
	unsigned int a = 0;
	int b = hex.length() - 1;
	for (; b >= 0; a++, b--)
	{
		if (hex[b] >= '0' && hex[b] <= '9')
		{
			value += (hex[b] - '0') * (1 << (a * 4));
		}
		else
		{
			switch (hex[b])
			{
			case 'A':
			case 'a':
				value += 10 * (1 << (a * 4));
				break;

			case 'B':
			case 'b':
				value += 11 * (1 << (a * 4));
				break;

			case 'C':
			case 'c':
				value += 12 * (1 << (a * 4));
				break;

			case 'D':
			case 'd':
				value += 13 * (1 << (a * 4));
				break;

			case 'E':
			case 'e':
				value += 14 * (1 << (a * 4));
				break;

			case 'F':
			case 'f':
				value += 15 * (1 << (a * 4));
				break;

			default:
				break;
			}
		}
	}

	return value;
}


string JOString::iToa( int value )
{
	return JOString::formatString("%d", value);
}

string JOString::llToa( long long value )
{
	return formatString("%lld", value);
}

string JOString::boolToa( bool value )
{
	if (value)
	{
		return "true";
	}
	return "false";
}

string JOString::floatToa( float value )
{
	return formatString("%f", value);
}

string JOString::doubleToa( double value )
{
	return formatString("%lf", value);
}
//////////////////////////////////////////////////////////////////////////


unsigned char JOString::toUChat(const char* str)
{
	unsigned char value = 0;
	if (sscanf(str, "%d", &value) == 1) {
		return value;
	}
	return 0;
}

char JOString::toChat(const char* str)
{
	char value = 0;
	if (sscanf(str, "%d", &value) == 1) {
		return value;
	}
	return 0;
}

int JOString::toInt( const char* str )
{
	int value = 0;
	if ( sscanf( str, "%d", &value ) == 1 ) {
		return value;
	}
	return 0;
}

unsigned JOString::toUnsigned( const char* str )
{
	unsigned value = 0;
	if ( sscanf( str, "%u", &value ) == 1 ) {
		return value;
	}
	return 0;
}

bool JOString::toBool( const char* str )
{	
	bool value = false;
	string testStr = str;
	toLowerCase(testStr);
	if ( isEqual( testStr.c_str(), "true" ) ) {
		value = true;		
	}	
	return value;
}

float JOString::toFloat( const char* str )
{
	float value = 0.0f;
	if ( sscanf( str, "%f", &value ) == 1 ) {
		return value;
	}
	return 0.0f;
}

double JOString::toDouble( const char* str )
{
	double value = 0;
	if ( sscanf( str, "%lf", &value ) == 1 ) {
		return value;
	}
	return 0;
}

long long JOString::toLongLong( const char* str )
{
	long long value = 0;
	if (sscanf( str, "%lld", &value) == 1){
		return value;
	}
	return 0;
}

std::string JOString::data2String(const unsigned char* pData, unsigned int len)
{
	const unsigned int desSize = len * 2 + len / 4 + 2;
	char* des = new char[desSize];
	memset(des, 0, desSize);
	for (unsigned int i = 0; i < len; i++) {
		sprintf(des, "%s%.2x", des, pData[i]);
		if ((i + 1) % 4 == 0 && i < len - 1) {
			sprintf(des, "%s ", des);
		}
	}
	std::string desc = "<";
	desc.append(des);
	desc.append(">");
	//delete des;
	return desc;
}



NS_JOFW_END