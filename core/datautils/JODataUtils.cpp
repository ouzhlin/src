//
//  GlobalData.cpp
//  Twilight-cocos2dx
//
//  Created by mac on 12-7-6.
//  Copyright (c) 2012??? __MyCompanyName__. All rights reserved.
//

#include "core/datautils/JODataUtils.h"
#include "core/datautils/JOData.h"
#include "utils/JOLog.h"
#include <iostream>

NS_JOFW_BEGIN

short JODataUtils::shortFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    if (buff == nullptr)
    {
        LOG_ERROR("JODataUtils","byte array is null!");
        return 0;
    }
    short r = 0;
    
    if (bigendian)
        for (unsigned int i = 0; i < 2; i++) {
            r <<= 8;
            r |= (buff[i] & 0x00ff);
        }
    
    else
        for (int i = 1; i >= 0; i--) {
            r <<= 8;
            r |= (buff[i] & 0x00ff);
        }
    
    return r;
}
void JODataUtils::dataFormShort(short s, unsigned char* buff, bool bigendian/*=false*/)
{
    if (bigendian)
        for (int i = 1; i >= 0; i--) {
            buff[i] = (unsigned char) (s & 0x00ff);
            s >>= 8;
        }
    
    else
		for (unsigned int i = 0; i < 2; i++) {
            buff[i] = (unsigned char) (s & 0x00ff);
            s >>= 8;
        }
}

unsigned short JODataUtils::unsignedShortFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    short r = shortFormData(buff, bigendian);
    if (r < 0) {
        return USHRT_MAX+1+r;
    }
    return r;
}
void JODataUtils::dataFormUnsignedShort(unsigned short s, unsigned char* buff, bool bigendian/*=false*/)
{
    if (s > SHRT_MAX) {
        dataFormShort(s-(USHRT_MAX+1), buff, bigendian);
    }
    dataFormShort(s, buff, bigendian);
}

// int32(4 bytes)
int JODataUtils::intFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    if (buff == NULL)
    {
        LOG_ERROR("JODataUtils","byte array is null!");
        return 0;
    }
    
    int r = 0;
    
    if (bigendian)
        for (int i = 0; i < 4; i++) {
            r <<= 8;
            r |= (buff[i] & 0x000000ff);
        }
    else
        for (int i = 3; i >= 0; i--) {
            r <<= 8;
            r |= (buff[i] & 0x000000ff);
        }
    
    return r;
}
void JODataUtils::dataFormInt(int n, unsigned char* buff, bool bigendian/*=false*/)
{
    if (bigendian)
        for (int i = 3; i >= 0; i--) {
            buff[i] = (unsigned char) (n & 0x000000ff);
            n >>= 8;
        }
    else
		for (unsigned int i = 0; i < 4; i++) {
            buff[i] = (unsigned char) (n & 0x000000ff);
            n >>= 8;
        }
}

unsigned int JODataUtils::unsignedIntFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    int r = intFormData(buff, bigendian);
    if (r < 0) {
        return UINT_MAX+1+r;
    }
    return r;
}
void JODataUtils::dataFormUnsignedInt(unsigned int n, unsigned char* buff, bool bigendian/*=false*/)
{
    if (n > INT_MAX) {
        dataFormInt(n-(UINT_MAX+1), buff, bigendian);
    }
    dataFormInt(n, buff, bigendian);
}

float JODataUtils::floatFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    if (buff == NULL)
    {
        LOG_ERROR("JODataUtils","byte array is null!");
        return 0;
    }
    
    float f = 0.0f;
    ::memcpy(&f, buff, sizeof(float));
    
    if (bigendian)
    {
        unsigned int temp = NET2HOST_32( *( (unsigned int*)&f ) );
        return *((float*)&temp);
    }
    
    return f;
}
void JODataUtils::dataFormfloat(float f, unsigned char* buff, bool bigendian/*=false*/)
{
    if (bigendian){
        unsigned int temp = HOST2NET_32( *( (unsigned int*)&f ) );
        memcpy(buff, &temp, sizeof(float));
    }
    else
        memcpy(buff, &f, sizeof(float));
}

// int64(8 bytes)
long long JODataUtils::longLongFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    if (buff == NULL)
    {
        LOG_ERROR("JODataUtils","byte array is null!");
        return 0;
    }
    
    long long r = 0;
    
    if (bigendian)
		for (unsigned int i = 0; i < 8; i++) {
            r <<= 8;
            r |= (buff[i] & 0x00000000000000ff);
        }
    else
        for (int i = 7; i >= 0; i--) {
            r <<= 8;
            r |= (buff[i] & 0x00000000000000ff);
        }
    
    return r;
}
void JODataUtils::dataFormLongLong(long long l, unsigned char* buff, bool bigendian/*=false*/)
{    
    if (bigendian)
        for (int i = 7; i >= 0; i--) {
            buff[i] = (unsigned char) (l & 0x00000000000000ff);
            l >>= 8;
        }
    else
		for (unsigned int i = 0; i < 8; i++) {
            buff[i] = (unsigned char) (l & 0x00000000000000ff);
            l >>= 8;
        }
}

unsigned long long JODataUtils::unsignedLongLongFormData(unsigned char* buff, bool bigendian/*=false*/)
{
    long long r = longLongFormData(buff, bigendian);
    if (r < 0) {
        return ULONG_LONG_MAX+1+r;
    }
    return r;
}
void JODataUtils::dataFormUnsignedLongLong(unsigned long long l, unsigned char* buff, bool bigendian/*=false*/)
{
    if (l > LONG_LONG_MAX) {
        return dataFormLongLong(l-(ULONG_LONG_MAX+1), buff, bigendian);
    }
    return dataFormLongLong(l, buff, bigendian);
}

short JODataUtils::shortFormData(JOData* data, bool bigendian/*=false*/)
{
    return shortFormData((unsigned char*)data->bytes(), bigendian);
    /*
	if (data == NULL)
	{
		LOG_ERROR("JODataUtils","byte array is null!");
		return 0;
	}
	if (data->length() > 2)
	{
		LOG_ERROR("JODataUtils","byte array size > 2 !");
		return 0;
	}
	
    short r = 0;
    
    if (bigendian)
        for (int i = 0; i < data->length(); i++) {
            r <<= 8;
            r |= (data->bytes()[i] & 0x00ff);
        }
        
    else
        for (int i = data->length() - 1; i >= 0; i--) {
            r <<= 8;
            r |= (data->bytes()[i] & 0x00ff);
        }
    
    return r;
     */
}

JOData* JODataUtils::dataFormShort(short s, bool bigendian/*=false*/)
{
    unsigned char* buf = new unsigned char[2];
    
    if (bigendian)
        for (int i = 1; i >= 0; i--) {
            buf[i] = (unsigned char) (s & 0x00ff);
            s >>= 8;
        }
    
    else
		for (unsigned int i = 0; i < 2; i++) {
            buf[i] = (unsigned char) (s & 0x00ff);
            s >>= 8;
        }
    JOData *d = new JOData(buf, 2);
    delete [] buf;
    return d;
}

unsigned short JODataUtils::unsignedShortFormData(JOData* data, bool bigendian/*=false*/)
{
    return unsignedShortFormData((unsigned char*)data->bytes(), bigendian);
    /*
    short r = shortFormData(data, bigendian);
    if (r < 0) {
        return USHRT_MAX+1+r;
    }
    return r;
     */
}
JOData* JODataUtils::dataFormUnsignedShort(unsigned short s, bool bigendian/*=false*/)
{
    if (s > SHRT_MAX) {
        return dataFormShort(s-(USHRT_MAX+1), bigendian);
    }
    return dataFormShort(s, bigendian);
}

int JODataUtils::intFormData(JOData* data, bool bigendian/*=false*/)
{
    return intFormData((unsigned char*)data->bytes(), bigendian);
    /*
	if (data == NULL)
	{
		LOG_ERROR("JODataUtils","byte array is null!");
		return 0;
	}
	if (data->length() > 4)
	{
		LOG_ERROR("JODataUtils", "byte array size > 4 !");
		return 0;
	}
	    
    int r = 0;
    
    if (bigendian)
        for (int i = 0; i < data->length(); i++) {
            r <<= 8;
            r |= (data->bytes()[i] & 0x000000ff);
        }
    else
        for (int i = data->length() - 1; i >= 0; i--) {
            r <<= 8;
            r |= (data->bytes()[i] & 0x000000ff);
        }
    
    return r;
     */
}

JOData* JODataUtils::dataFormInt(int n, bool bigendian/*=false*/)
{
    unsigned char* buf = new unsigned char[4];
    
    if (bigendian)
        for (int i = 3; i >= 0; i--) {
            buf[i] = (unsigned char) (n & 0x000000ff);
            n >>= 8;
        }
    else
		for (unsigned int i = 0; i < 4; i++) {
            buf[i] = (unsigned char) (n & 0x000000ff);
            n >>= 8;
        }
    
    JOData *d = new JOData(buf, 4);
    delete [] buf;
    return d;
}


float JODataUtils::floatFormData( JOData* data, bool bigendian/*=false*/ )
{
    return floatFormData((unsigned char*)data->bytes(), bigendian);
    /*
	if (data == NULL)
	{
		LOG_ERROR("JODataUtils","byte array is null!");
		return 0;
	}
	if (data->length() > 4)
	{
		LOG_ERROR("JODataUtils","byte array size > 4 !");
		return 0;
	}
	
	float f = 0.0f;
	::memcpy(&f, data->bytes(), sizeof(float));	

	if (bigendian)
	{
		unsigned int temp = NET2HOST_32( *( (unsigned int*)&f ) );
		return *((float*)&temp);
	}
		
	return f;
     */
}

JOData* JODataUtils::dataFormfloat( float f, bool bigendian/*=false*/ )
{
	unsigned char* buf = new unsigned char[sizeof(float)];

	if (bigendian){
		unsigned int temp = HOST2NET_32( *( (unsigned int*)&f ) );
		memcpy(buf, &temp, sizeof(float));	
	}
	else
		memcpy(buf, &f, sizeof(float));

	JOData *d = new JOData(buf, 4);
	delete [] buf;
	return d;
}


unsigned int JODataUtils::unsignedIntFormData(JOData* data, bool bigendian/*=false*/)
{
    return unsignedIntFormData((unsigned char*)data->bytes(), bigendian);
    /*
    int r = intFormData(data, bigendian);
    if (r < 0) {
        return UINT_MAX+1+r;
    }
    return r;
     */
}

JOData* JODataUtils::dataFormUnsignedInt(unsigned int n, bool bigendian/*=false*/)
{
    if (n > INT_MAX) {
        return dataFormInt(n-(UINT_MAX+1), bigendian);
    }
    return dataFormInt(n, bigendian);
}

long long JODataUtils::longLongFormData(JOData* data, bool bigendian/*=false*/)
{
    return longLongFormData((unsigned char*)data->bytes(), bigendian);
    /*
	if (data == NULL)
	{
		LOG_ERROR("JODataUtils","byte array is null!");
		return 0;
	}
	if (data->length() > 8)
	{
		LOG_ERROR("JODataUtils","byte array size > 8 !");
		return 0;
	}
        
    long long r = 0;
    
    if (bigendian)
        for (int i = 0; i < data->length(); i++) {
            r <<= 8;
            r |= (data->bytes()[i] & 0x00000000000000ff);
        }
    else
        for (int i = data->length() - 1; i >= 0; i--) {
            r <<= 8;
            r |= (data->bytes()[i] & 0x00000000000000ff);
        }
    
    return r;
     */
}

JOData* JODataUtils::dataFormLongLong(long long l, bool bigendian/*=false*/)
{
    unsigned char* buf = new unsigned char[8];
    
    if (bigendian)
        for (int i = 7; i >= 0; i--) {
            buf[i] = (unsigned char) (l & 0x00000000000000ff);
            l >>= 8;
        }
    else
		for (unsigned int i = 0; i < 8; i++) {
            buf[i] = (unsigned char) (l & 0x00000000000000ff);
            l >>= 8;
        }
    
    JOData *d = new JOData(buf, 8);
    delete [] buf;
    return d;
}

unsigned long long JODataUtils::unsignedLongLongFormData(JOData* data, bool bigendian/*=false*/)
{
    return unsignedLongLongFormData((unsigned char*)data->bytes(), bigendian);
    /*
    long long r = longLongFormData(data, bigendian);
    if (r < 0) {
        return ULONG_LONG_MAX+1+r;
    }
    return r;
     */
}

JOData* JODataUtils::dataFormUnsignedLongLong(unsigned long long l, bool bigendian/*=false*/)
{
    if (l > LONG_LONG_MAX) {
        return dataFormLongLong(l-(ULONG_LONG_MAX+1), bigendian);
    }
    return dataFormLongLong(l, bigendian);
}

NS_JOFW_END