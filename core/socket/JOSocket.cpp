
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

#include "utils/JOString.h"
#include "utils/JOLog.h"
#include "manager/JOTickMgr.h"

#ifndef _WIN32

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#else

#ifndef __MINGW32__
#include <WinSock2.h>
#else

#undef _WINSOCKAPI_
#include <winsock2.h>
#endif //__MINGW32__

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

#pragma mark -
#pragma mark JOSocket

static std::thread t1;
static std::thread t2;
static std::thread t3;

JOSocket::JOSocket(short id, JOSocketDelegate* socketDelegate)
	:m_socketId(id)
	, m_pDelegate(socketDelegate)
	, m_nSocketHandle(-1)
	, bConnect_(false)
{    
    m_recv_buffer = new JORingBuffer<JODataCoder *>(100);
    m_send_buffer = new JORingBuffer<JODataCoder *>(100);
}

JOSocket::JOSocket()
	:m_socketId(-1)
	, m_pDelegate(nullptr)
	, m_nSocketHandle(-1)
	, bConnect_(false)
{
	m_recv_buffer = new JORingBuffer<JODataCoder *>(100);
	m_send_buffer = new JORingBuffer<JODataCoder *>(100);
}

JOSocket::~JOSocket()
{
	disconnect();
}

void JOSocket::setDelegate(JOSocketDelegate* delegate)
{
	m_delegateLock.lock();
	m_pDelegate = delegate;
	m_delegateLock.unlock();
}

JOSocketDelegate* JOSocket::getDelegate()
{
	m_delegateLock.lock();
	return m_pDelegate;
	m_delegateLock.unlock();
}

bool JOSocket::isConnect()
{
    return bConnect_;
}

void JOSocket::connectThread( )
{
	JOLockGuard tempLock(m_socketLock);
	bool bConnectFail = false;
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
	if (bConnectFail)
	{
		JOTickMgr::Instance()->runInMainThread([&, this]{
			if (m_pDelegate){
				m_pDelegate->onSocketConnectFail(this);
			}
			this->disconnect();
		});
		return;
	}
    
	JOTickMgr::Instance()->runInMainThread([&, this]{
		LOG_DEBUG("JOSocket", "Client connect OK! IP: %s:%d ", m_szIP.c_str(), m_nPort);
		this->_createStream();
	});
}

int JOSocket::connect(const char* ip, unsigned int port, bool async)
{ 
    m_szIP = ip;
    m_nPort = port;
    
    if (m_pDelegate) {
        m_pDelegate->onSocketWillConnect(this);
    }

	if (async) {
		//clearBuffer();
		t1 = std::thread(&JOSocket::connectThread, this);        
		//m_threadConnect = new std::thread(&JOSocket::connectThread, this);
		return 0;
	}
//     
	connectThread();
    return 0;
}

void JOSocket::_createStream()
{	
	if (!JOSocket::m_bReadStreamCreated) {
		t2 = std::thread(&JOSocket::readStreamThread, this);
		//m_threadRead = new std::thread(&JOSocket::readStreamThread, this);
		JOSocket::m_bReadStreamCreated = true;
	}
	if (!JOSocket::m_bWriteStreamCreated) {
		t3 = std::thread(&JOSocket::writeStreamThread, this);
		//m_threadWrite = new std::thread(&JOSocket::writeStreamThread, this);
		JOSocket::m_bWriteStreamCreated = true;
	}
    bConnect_ = true;
	if (m_pDelegate) {
		m_pDelegate->onSocketConnected(this, m_szIP.c_str(), m_nPort);
	}
}

void JOSocket::closeWithError(JOSocketError error)
{
	if (m_pDelegate && m_nSocketHandle != 0)
	{		
		m_pDelegate->onSocketWithError(this, error);
	}	
	disconnect();
}

void JOSocket::disconnect()
{
	m_socketLock.lock();

	JOSocket::m_bWriteStreamCreated = false;
	JOSocket::m_bReadStreamCreated = false;
 
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
    if (m_pDelegate) {
        m_pDelegate->onSocketDidDisconnected(this);
    }	
	m_socketLock.unlock();
	clearBuffer();	
}

#pragma mark JOSocket Writing
void JOSocket::writeData(JODataCoder* dataCoder)
{
    if (m_send_buffer->avaliable()) {
        m_send_buffer->push(dataCoder);
		std::unique_lock<std::mutex> lk(m_sendMutex);
		lk.unlock();
		m_sendCV.notify_one();
		lk.lock();
    }
}

#pragma mark -
#pragma mark JOSocket Writestream
bool JOSocket::m_bWriteStreamCreated = false;

void JOSocket::writeStreamThread()
{
	m_bWriteStreamCreated = true;
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
				//handle = m_nSocketHandle;
				//m_socketLock.unlock();
				if (m_nSocketHandle < 1)
				{
                    m_bWriteStreamCreated = false;
                    JOTickMgr::Instance()->runInMainThread([&, this]{
                        this->closeWithError(kJOSocketWriteError);
                    });
                    m_socketLock.unlock();
					break;
				}
				ret = send(m_nSocketHandle, (char*)writeBuffer, writeBufferRequired, 0);
                m_socketLock.unlock();
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
					if (m_bWriteStreamCreated)
					{
						m_bWriteStreamCreated = false;
						JOTickMgr::Instance()->runInMainThread([&, this]{
							this->closeWithError(kJOSocketWriteError);
						});
					}
				}
			}
		}
		else
		{
			std::unique_lock<std::mutex> lk(m_sendMutex);
			m_sendCV.wait(lk);
		}
	}
	JO_SAFE_DELETE_ARRAY(writeBuffer);
}

#pragma mark -
#pragma mark JOSocket Readstream
bool JOSocket::m_bReadStreamCreated = false;

void JOSocket::readStreamThread()
{
    m_bReadStreamCreated = true;
	unsigned int headLen = sizeof(NetPacketHeader);
	unsigned int poolSizePlus = 1;
	unsigned int readBufferDone = 0;
    unsigned char* readBuffer = new unsigned char[READALL_CHUNKSIZE];
	unsigned char* partialBuffer = new unsigned char[READALL_CHUNKSIZE];
	memset(readBuffer, 0, READALL_CHUNKSIZE);
	int handle = -1;
	unsigned int recvLen = 0;
	unsigned int ret = 0;
    while (m_bReadStreamCreated) {
		THREAD_SLEEP(1000);        
		m_socketLock.lock();
		handle = m_nSocketHandle;
		m_socketLock.unlock();
		if (handle < 1)
		{
            m_bReadStreamCreated = false;
            JOTickMgr::Instance()->runInMainThread([&, this]{
                this->closeWithError(kJOSocketReadError);
            });
			break;
		}
		memset(partialBuffer, 0, READALL_CHUNKSIZE);
		ret = recv(handle, (char*)partialBuffer, READALL_CHUNKSIZE, 0);
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
				recvLen=0;
				JODataCoder* dataCoder = nullptr;
				m_delegateLock.lock();
				if (m_pDelegate)
				{
					//dataCoder = m_pDelegate->onSocketDidReadPartialData(this, readBuffer, readBufferDone, &recvLen);
					while (readBufferDone >= headLen && readBuffer)
					{
						dataCoder = m_pDelegate->onSocketDidReadPartialData(this, readBuffer, readBufferDone, &recvLen);

						if (recvLen > 0 && dataCoder)
						{
							m_recv_buffer->push(dataCoder);

							readBufferDone = readBufferDone - recvLen;
							memmove(readBuffer, readBuffer + recvLen, readBufferDone);

							JOTickMgr::Instance()->runInMainThread([&, this]{
								this->_popReadData();
							});
						}
					}
				}				
				m_delegateLock.unlock();
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
			if (m_bReadStreamCreated)
			{
				m_bReadStreamCreated = false;
				JOTickMgr::Instance()->runInMainThread([&, this]{
					this->closeWithError(kJOSocketReadError);
				});				
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

void JOSocket::clearBuffer()
{	
	m_bWriteStreamCreated = false;	
	m_bReadStreamCreated = false;
	std::unique_lock<std::mutex> lk(m_sendMutex);
	lk.unlock();
	m_sendCV.notify_all();
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
