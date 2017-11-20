
//  JOSocket.cpp
//  Twilight-cocos2dx
//
//  Created by James Ou on 12-6-25.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "core/socket/JOSocket.h"
#include <iostream>
#include "core/datautils/JODataPool.h"
#include "core/datautils/JOData.h"
#include "core/datautils/JODataCoder.h"
#include "core/datautils/JODataUtils.h"

#include "utils/JOString.h"
#include "utils/JOLog.h"
#include "manager/JOTickMgr.h"
#include "manager/JOSnMgr.h"

#ifndef _WIN32

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sys/types.h>
#include <arpa/inet.h>
#else

#ifndef __MINGW32__
#include <WinSock2.h>
#else

#undef _WINSOCKAPI_
#include <winsock2.h>
#endif //__MINGW32__

#pragma comment(lib, "ws2_32.lib")
#include <WS2tcpip.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <BaseTsd.h>
#include <WinSock2.h>

#ifndef __SSIZE_T
#define __SSIZE_T
typedef SSIZE_T ssize_t;
#endif // __SSIZE_T
#endif 

#include <Windows.h>
#include <BaseTsd.h>

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#endif

#include <string>

NS_JOFW_BEGIN

enum TLocalIPStack {
	ELocalIPStack_None = 0,
	ELocalIPStack_IPv4 = 1,
	ELocalIPStack_IPv6 = 2,
	ELocalIPStack_Dual = 3,
};
TLocalIPStack _getIpv()
{
	const char* ipaddr = "www.baidu.com";

	TLocalIPStack type;

	struct addrinfo *answer, hint, *curr;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = PF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	hint.ai_flags = MSG_CTRUNC | AI_ADDRCONFIG;
#else
	hint.ai_flags = AI_DEFAULT;
#endif

	int ret = getaddrinfo(ipaddr, "http", &hint, &answer);
	if (ret != 0) {
		type = ELocalIPStack_None;
		return type;
	}
	else
	{
		for (curr = answer; curr != NULL; curr = curr->ai_next) {
			switch (curr->ai_family)
			{
			case AF_UNSPEC:
				//do something here
				break;
			case AF_INET:
				type = ELocalIPStack_IPv4;
				break;
			case AF_INET6:
				type = ELocalIPStack_IPv6;
				break;
			default:
				break;
			}
			if (type != ELocalIPStack_None) break;
		}
	}

	if (answer) freeaddrinfo(answer);

	return type;
}


std::string _getIPv6Addr(const char *ip)
{
	char nIp[128];
	{
		struct addrinfo hints, *res, *res0;
		int error;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		hints.ai_flags = MSG_CTRUNC | AI_ADDRCONFIG;
#else
		hints.ai_flags = AI_DEFAULT;
#endif
		error = getaddrinfo(ip, "http", &hints, &res0);
		for (res = res0; res; res = res->ai_next)
		{
			inet_ntop(AF_INET6,
				&(((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr),
				nIp, 128);
			break;
		}
	}
	return nIp;
}

#pragma mark -
#pragma mark JOSocket

JOSocket::JOSocket(short id, JOSocketDelegate* socketDelegate)
	:m_socketId(id)
	, m_pDelegate(socketDelegate)
	, m_nSocketHandle(-1)
	, bConnect_(false)
	, m_isClosing(false)
	, m_isBigEndian(false)
	, m_bWriteStreamCreated(false)
	, m_bReadStreamCreated(false)
{    
    m_recv_buffer = new JORingBuffer<JODataCoder *>(100);
    m_send_buffer = new JORingBuffer<JODataCoder *>(100);
	m_sn = JOSnMgr::Instance()->getSn();
}

JOSocket::JOSocket()
	:m_socketId(-1)
	, m_pDelegate(nullptr)
	, m_nSocketHandle(-1)
	, bConnect_(false)
	, m_isClosing(false)
	, m_isBigEndian(false)
	, m_bWriteStreamCreated(false)
	, m_bReadStreamCreated(false)
{
	m_recv_buffer = new JORingBuffer<JODataCoder *>(100);
	m_send_buffer = new JORingBuffer<JODataCoder *>(100);
	m_sn = JOSnMgr::Instance()->getSn();
}

JOSocket::~JOSocket()
{
	disconnect();
	JOTickMgr::Instance()->cancelInMainRun(m_sn);
	JOSnMgr::Instance()->dispose(m_sn);
}

void JOSocket::setDelegate(JOSocketDelegate* delegate)
{
	m_pDelegate = delegate;
}

JOSocketDelegate* JOSocket::getDelegate()
{
	return m_pDelegate;
}

bool JOSocket::isConnect()
{
    return bConnect_;
}

void JOSocket::connectThread( )
{
	TLocalIPStack type = _getIpv();

	m_socketLock.lock();
	bool bConnectFail = false;
	if (type == ELocalIPStack_IPv6){
		std::string ip = _getIPv6Addr(m_szIP.c_str());

		struct in6_addr ipv6_addr = { 0 };
		int v6_r = inet_pton(AF_INET6, ip.c_str(), &ipv6_addr);

		struct sockaddr_in6 v6_addr = { 0 };
		v6_addr.sin6_family = AF_INET6;
		v6_addr.sin6_port = htons(m_nPort);
		v6_addr.sin6_addr = ipv6_addr;

		//socket connect
		m_nSocketHandle = socket(AF_INET6, SOCK_STREAM, 0);

		std::string v6_error;

		if (0 != ::connect(m_nSocketHandle, (sockaddr*)&v6_addr, 28))
		{
			string v6_error = strerror(errno);			
			LOG_ERROR("JOSocket", "failed to create socket %d [%s] ", m_socketId, v6_error.c_str());
			bConnectFail = true;
		}
	}
	else{
		while (true)
		{
#if _WIN32
			WSADATA		wsaData;
			WORD		wVersionRequested;

			wVersionRequested = MAKEWORD(2, 0);
			m_nSocketHandle = INVALID_SOCKET;
			int iStatus = WSAStartup(wVersionRequested, &wsaData);
#endif

			struct sockaddr_in sa;
			struct hostent* hp;

			hp = (hostent*)gethostbyname(m_szIP.c_str());
			if (!hp){
				bConnectFail = true;
				break;
			}
			memset(&sa, 0, sizeof(sa));
			//memcpy((char*)&sa.sin_addr, hp->h_addr, hp->h_length);
			sa.sin_addr = *((struct in_addr*) hp->h_addr);
			sa.sin_family = AF_INET;//hp->h_addrtype;
			sa.sin_port = htons(m_nPort);
			//sa.sin_addr.s_addr = htons(INADDR_ANY);

			// 	m_nSocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			m_nSocketHandle = socket(AF_INET, SOCK_STREAM, 0);
#if _WIN32		
			if (m_nSocketHandle == INVALID_SOCKET) {
				WSACleanup();
				bConnectFail = true;
				break;
			}
#endif
			if (m_nSocketHandle < 0){
				LOG_ERROR("JOSocket", "failed to create socket %d ", m_socketId);
				bConnectFail = true;
				break;
			}
			if (::connect(m_nSocketHandle, (sockaddr*)&sa, sizeof(sockaddr)) == -1){
				LOG_ERROR("JOSocket", "failed to connect socket %d ", m_socketId);
				bConnectFail = true;
			}
			break;
		}
	}
	m_socketLock.unlock();

	if (bConnectFail)
	{
		JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
			if (m_pDelegate){
				m_pDelegate->onSocketConnectFail(this);
			}
			this->disconnect();
		});
		return;
	}
	JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
		LOG_DEBUG("JOSocket", "Client connect OK! IP: %s:%d ", m_szIP.c_str(), m_nPort);
		this->_createStream();
	});
}

void JOSocket::connect(std::string& ip, unsigned int port, bool async)
{ 
    m_szIP = ip;
    m_nPort = port;
    
    if (m_pDelegate) {
        m_pDelegate->onSocketWillConnect(this);
    }

	if (async) {
		JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
			t1 = std::thread(&JOSocket::connectThread, this);
		});
		//t1 = std::thread(&JOSocket::connectThread, this);
		//m_threadConnect = new std::thread(&JOSocket::connectThread, this);
		return;
	}
	connectThread();
}

void JOSocket::_createStream()
{	
	if (!m_bReadStreamCreated) {
		m_bReadStreamCreated = true;
		t2 = std::thread(&JOSocket::readStreamThread, this);
		//m_threadRead = new std::thread(&JOSocket::readStreamThread, this);
	}
	if (!m_bWriteStreamCreated) {
		m_bWriteStreamCreated = true;
		t3 = std::thread(&JOSocket::writeStreamThread, this);
		//m_threadWrite = new std::thread(&JOSocket::writeStreamThread, this);
	}
    bConnect_ = true;
	if (m_pDelegate) {
		m_pDelegate->onSocketConnected(this, m_szIP.c_str(), m_nPort);
	}
}

void JOSocket::closeWithError(JOSocketError error)
{
	if (m_pDelegate)
	{		
		m_pDelegate->onSocketWithError(this, error);
	}
	disconnect();
}

void JOSocket::disconnect()
{
	m_isClosing = true;

	m_bWriteStreamCreated = false;
	m_bReadStreamCreated = false;
	
	m_socketLock.lock();
	// Close sockets.
	if (m_nSocketHandle != 0)
	{		
#if _WIN32
		shutdown(m_nSocketHandle, SD_BOTH);
		closesocket(m_nSocketHandle);
#else
		shutdown(m_nSocketHandle, SHUT_RDWR);
		::close(m_nSocketHandle);
#endif
        m_nSocketHandle = 0;
	}
    bConnect_ = false;
	m_socketLock.unlock();

	_clearBuffer();
    if (m_pDelegate) {
        m_pDelegate->onSocketDidDisconnected(this);
    }	
	m_isClosing = false;
}

#pragma mark JOSocket Writing
void JOSocket::writeData(JODataCoder* dataCoder)
{
    if (m_send_buffer->avaliable()) {
        m_send_buffer->push(dataCoder);
		
		/*std::unique_lock<std::mutex> lk(m_sendMutex);
		lk.unlock();
		m_sendCV.notify_one();
		lk.lock();*/
		
    }
}

#pragma mark -
#pragma mark JOSocket Writestream

void JOSocket::writeStreamThread()
{
	unsigned int poolSizePlus = 1;
	unsigned int writeBufferRequired = 0;
	unsigned char* writeBuffer = new unsigned char[WRITE_CHUNKSIZE];
	bool chance = false;
	unsigned int dataLen = 0;
	int ret = 0;

	while (m_bWriteStreamCreated)
	{
		THREAD_SLEEP(500);
		if (!m_send_buffer->empty()) {
			JODataCoder *dataCoder = m_send_buffer->pop();
			JOData *data = dataCoder->netPack();
            dataLen = data->length();
			chance = false;
			while (dataLen > poolSizePlus*WRITE_CHUNKSIZE - writeBufferRequired)
			{
				++poolSizePlus;
				chance = true;
			}
			if (chance)
			{
				unsigned char* newBuffer = new unsigned char[poolSizePlus * WRITE_CHUNKSIZE];

				memcpy(newBuffer, writeBuffer, writeBufferRequired);
				JO_SAFE_DELETE_ARRAY(writeBuffer);
				writeBuffer = newBuffer;
			}

			memcpy(writeBuffer + writeBufferRequired, data->bytes(), dataLen);
			writeBufferRequired += dataLen;
			//delete dataCoder;
			JODataPool::Instance()->recover(dataCoder, true);

			if (writeBufferRequired > 0)
			{
				m_socketLock.lock();
				if (m_nSocketHandle < 1)
				{
					m_socketLock.unlock();
                    m_bWriteStreamCreated = false;
					JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
                        this->closeWithError(kJOSocketWriteError);
                    });
					break;
				}
				m_socketLock.unlock();
				
				ret = send(m_nSocketHandle, (char*)writeBuffer, writeBufferRequired, 0);
                
				if (ret > 0)
				{
					if (ret == writeBufferRequired)
					{
						memset(writeBuffer, 0, writeBufferRequired);
						writeBufferRequired = 0;
					}
					else {
						writeBufferRequired -= ret;
						memcpy(writeBuffer, writeBuffer + ret, writeBufferRequired);
					}
				}
				else {
					if (m_bWriteStreamCreated && m_isClosing==false)
					{
						m_bWriteStreamCreated = false;
						JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
							this->closeWithError(kJOSocketWriteError);
						});
						break;
					}
				}
			}
		}
		/*else if (m_bWriteStreamCreated && m_isClosing == false)
		{
			std::unique_lock<std::mutex> lk(m_sendMutex);
			m_sendCV.wait(lk);
		}*/
	}
	JO_SAFE_DELETE_ARRAY(writeBuffer);
}

#pragma mark -
#pragma mark JOSocket Readstream

void JOSocket::readStreamThread()
{
	unsigned int headLen = sizeof(NetPacketHeader);
	unsigned int poolSizePlus = 1;
	unsigned int readBufferDone = 0;
    unsigned char* readBuffer = new unsigned char[READALL_CHUNKSIZE];
	unsigned char* partialBuffer = new unsigned char[READALL_CHUNKSIZE];
	memset(readBuffer, 0, READALL_CHUNKSIZE);

	unsigned int recvLen = 0;
	int ret = 0;
    while (m_bReadStreamCreated) {
		THREAD_SLEEP(500);        
		m_socketLock.lock();
		if (m_nSocketHandle < 1)
		{
            m_bReadStreamCreated = false;
			m_socketLock.unlock();
			JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
                this->closeWithError(kJOSocketReadError);
            });
			break;
		}
		m_socketLock.unlock();
		memset(partialBuffer, 0, READALL_CHUNKSIZE);
		ret = recv(m_nSocketHandle, (char*)partialBuffer, READALL_CHUNKSIZE, 0);
		
		if (ret > 0 ) {
			
			// if read buffer full, extend buffer size
			if (readBufferDone + ret >= poolSizePlus*READALL_CHUNKSIZE)
			{				
				poolSizePlus = (readBufferDone+ret)/READALL_CHUNKSIZE+1;
				unsigned char* newBuffer = new unsigned char[poolSizePlus*READALL_CHUNKSIZE];
				memcpy(newBuffer, readBuffer, readBufferDone);
				JO_SAFE_DELETE_ARRAY(readBuffer);
				readBuffer = newBuffer;
			}
            
			memcpy(readBuffer + readBufferDone, partialBuffer, ret);
			readBufferDone += ret;
			if (m_recv_buffer->avaliable()){
				
				//dataCoder = m_pDelegate->onSocketDidReadPartialData(this, readBuffer, readBufferDone, &recvLen);
				while (readBufferDone >= headLen && readBuffer)
				{
					NetPacketHeader* pHead = (NetPacketHeader*)(readBuffer);
					unsigned int nPacketSize = pHead->wDataSize;
					if (m_isBigEndian)
					{
						nPacketSize = NET2HOST_16(pHead->wDataSize);
					}

					unsigned int sizeOccupy = sizeof(pHead->wDataSize);
					recvLen = sizeOccupy + nPacketSize;
					JODataCoder* dataCoder = nullptr;
					if (readBufferDone < recvLen)
					{
						LOG_ERROR("JOSocketMgr", "net data pack len error !!!");
						recvLen = 0;
					}
					else{
						unsigned int op = JODataUtils::unsignedShortFormData(readBuffer + sizeOccupy, m_isBigEndian);
						dataCoder = JODataPool::Instance()->getDataCoder(true, m_isBigEndian, true);
						dataCoder->setOp(op);
						if (nPacketSize>2)
							dataCoder->init(readBuffer + sizeOccupy + 2, nPacketSize - 2);
					}

					if (recvLen > 0 && dataCoder)
					{
						m_recv_buffer->push(dataCoder);

						readBufferDone = readBufferDone - recvLen;
						memmove(readBuffer, readBuffer + recvLen, readBufferDone);

						JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
							this->_popReadData();
						});
					}
				}
							
				/*
				if (recvLen > 0 && dataCoder)
				{
					m_recv_buffer->push(dataCoder);

					readBufferDone = readBufferDone - recvLen;
					memmove(readBuffer, readBuffer + recvLen, readBufferDone);

					JOTickMgr::Instance()->runInMainThread([&, this]{
						this->_popReadData();
					});
				}
				*/
			}
        }
        // socket disconnect
        else if (ret <= 0) {
			if (m_bReadStreamCreated && m_isClosing==false)
			{
				m_bReadStreamCreated = false;
				JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
					this->closeWithError(kJOSocketReadError);
				});
				break;
			}
        }
    }
	JO_SAFE_DELETE_ARRAY(readBuffer);
    JO_SAFE_DELETE_ARRAY(partialBuffer);
}

void JOSocket::_popReadData()
{    
    if (m_recv_buffer->empty() == false)
    {
        JODataCoder* dataCoder = m_recv_buffer->pop();
        if (dataCoder && m_pDelegate)
        {
            m_pDelegate->onSocketDidReadData(this,dataCoder);
        }
    }
}

void JOSocket::_clearBuffer()
{	
	/*std::unique_lock<std::mutex> lk(m_sendMutex);
	lk.unlock();
	m_sendCV.notify_one();*/
    while (m_recv_buffer->empty() == false)
    {
        JODataCoder* dataCoder = m_recv_buffer->pop();
		JODataPool::Instance()->recover(dataCoder, true);
    }

    while (m_send_buffer->empty() == false)
    {
        JODataCoder* dataCoder = m_send_buffer->pop();
		JODataPool::Instance()->recover(dataCoder, true);
    }
	_clearThread();
}

void JOSocket::_clearThread()
{
	if (t1.joinable())
	{
		t1.join();
	}
	if (t2.joinable())
	{
		t2.join();
	}
	if (t3.joinable())
	{
		t3.join();
	}
}




/*
int keepalive = 1;             // ????????????
int keepidle = 3;        // ????????????????????????????????????
int keepintvl = 2;        // ?????????????????????????????????
int keepcnt = 2;        // ???????????????????????????

#ifndef _WIN32
if (setsockopt(socketObj->m_nSocketHandle, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof (keepalive)) < 0)
{
printf("fail to set SO_KEEPALIVE");
socketObj->m_bExecSocketConnectFailOnMainThread = true;
return NULL;
}
//andriod???ios????????????????????????????????????
int macosKeepIdle = 0x10;
#ifdef TCP_KEEPIDLE
macosKeepIdle = TCP_KEEPIDLE;
#elif defined TCP_KEEPALIVE
macosKeepIdle = TCP_KEEPALIVE;
#endif
if (setsockopt(socketObj->m_nSocketHandle, IPPROTO_TCP, macosKeepIdle, (void *) &keepidle, sizeof (keepidle)) < 0)
{
printf("fail to set SO_KEEPIDLE");
socketObj->m_bExecSocketConnectFailOnMainThread = true;
return NULL;
}
if (setsockopt(socketObj->m_nSocketHandle, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof (keepintvl)) < 0)
{
printf("fail to set SO_KEEPINTVL");
socketObj->m_bExecSocketConnectFailOnMainThread = true;
return NULL;
}
if (setsockopt(socketObj->m_nSocketHandle, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof (keepcnt)) < 0)
{
printf("fail to set SO_KEEPALIVE");
socketObj->m_bExecSocketConnectFailOnMainThread = true;
return NULL;
}
#endif
*/

NS_JOFW_END
