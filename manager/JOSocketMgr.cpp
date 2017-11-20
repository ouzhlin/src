//
//  GlobalData.cpp
//  Twilight-cocos2dx
//
//  Created by mac on 12-7-6.
//  Copyright (c) 2012??? __MyCompanyName__. All rights reserved.
//
#include "manager/JOSocketMgr.h"
#include "core/datautils/JOData.h"
#include "core/datautils/JODataCoder.h"
#include "core/datautils/JODataUtils.h"
#include "core/datautils/JODataPool.h"
#include "utils/JOString.h"
#include "utils/JOLog.h"
#include "manager/JOCachePoolMgr.h"
#include "module/event/JOEventDispatcher.h"
NS_JOFW_BEGIN


JOSocketMgr::JOSocketMgr()
: m_mainSocketId(1)
, m_bDebug(false)
{

}

JOSocketMgr::~JOSocketMgr()
{
	_disconnectAll();
}

void JOSocketMgr::initMainSocketId(int id)
{
	SOCKET_MAP::iterator itr = socket_map.find(m_mainSocketId);
	if (itr != socket_map.end())
	{
		itr->second->setSocketId(id);
	}
	m_mainSocketId = id;
}

void JOSocketMgr::connect(std::string& ip, unsigned int port, bool beAsyn, bool isBigEndian, int socketId /*= -1*/)
{
	JOSocket* st = _getSocket(socketId);
	if (st)
	{
		if (st->isConnect())
		{
			LOG_WARN("JOSocketMgr", "Socket[%d] is connecting!!", socketId);
			disconnect(socketId);
			st = _getSocket(socketId);
		}
		st->setBigEndian(isBigEndian);
		st->connect(ip, port, beAsyn);
	}
}

bool JOSocketMgr::disconnect(int socketId /*= -1*/)
{
	JOSocket* st = _findSocket(socketId);
	if (st)
	{
		st->setDelegate(nullptr);		
		st->disconnect();	
		POOL_RECOVER(st, JOSocket, "JOSocketMgr");
		socket_map.erase(socketId);
		//st->setDelegate(this);
		return true;
	}
	return false;
}


void JOSocketMgr::_disconnectAll()
{
	SOCKET_MAP::iterator itr = socket_map.begin();
	while (itr != socket_map.end())
	{
		itr->second->setDelegate(nullptr);
		itr->second->disconnect();
		//itr->second->clearBuffer();
		POOL_RECOVER(itr->second, JOSocket, "JOSocketMgr");
		++itr;
	}
	socket_map.clear();
}


void JOSocketMgr::sendMessage(JODataCoder* dataCoder, int socketId /*= -1*/)
{
	JOSocket* st = _findSocket(socketId);
	if (st)
	{
		if (st->isConnect() == false)
		{
			LOG_WARN("JOSocketMgr", "Socket[%d] not connect can't sendMessage!!", socketId);
			return;
		}

		if (m_bDebug)
		{
			LOG_INFO("JOSocketMgr", "(data not include lenght) \nsend: op = %d\nlen: = %d\ndata = %s", dataCoder->getOp(), dataCoder->length(), dataCoder->description().c_str());
		}
		st->writeData(dataCoder);
	}
	else
	{
		LOG_WARN("JOSocketMgr", "Socket[%d] not finded!!", socketId);
	}
}

JOSocket* JOSocketMgr::_getSocket(int id)
{
	JOSocket* st = _findSocket(id);
	if (st==nullptr)
	{
		st = POOL_GET(JOSocket, "JOSocketMgr");// new JOSocket(id, this);
		st->setSocketId(id);
		st->setDelegate(this);		
		socket_map[id] = st;
	}
	return st;
}

JOSocket* JOSocketMgr::_findSocket(int id)
{
	SOCKET_MAP::iterator itr = socket_map.find(id);
	if (itr != socket_map.end())
	{
		return itr->second;
	}
	return nullptr;
}

void JOSocketMgr::onSocketWillConnect(JOSocket* socket)
{

}

void JOSocketMgr::onSocketConnected(JOSocket* socket, const char* ip, unsigned int port)
{
	JOEventDispatcher::Instance()->dispatchEvent(SOCKET_EVENT_CONNECT);
}

void JOSocketMgr::onSocketConnectFail(JOSocket* socket)
{
	JOEventDispatcher::Instance()->dispatchEvent(SOCKET_EVENT_CONNECT_FAIL);
}

void JOSocketMgr::onSocketDidDisconnected(JOSocket* socket)
{
	JOEventDispatcher::Instance()->dispatchEvent(SOCKET_EVENT_DISCONNECT);
}

void JOSocketMgr::onSocketDidReadData(JOSocket* socket, JODataCoder* dataCoder)
{
	if (dataCoder) {
		if (m_bDebug){		
			LOG_INFO("JOSocketMgr", "\nrecv: op = %d\nlen: = %d\ndata = %s", dataCoder->getOp(), dataCoder->length(), dataCoder->description().c_str());
		}
		JODataCoder* coder = JODataPool::Instance()->getDataCoder();
		coder->putShort(dataCoder->getOp());
		JOEventDispatcher::Instance()->g2cAction(dataCoder, socket->getSocketId());
		JOEventDispatcher::Instance()->dispatchEvent(SOCKET_EVENT_RECV_OP, coder);

	}
	else{
		LOG_ERROR("JOSocketMgr", "message == NULL");
	}
}

/*
struct NetPacketHeader
{
	unsigned short wDataSize;
	short wOpcode; // -10 ~ 10
};
*/

void JOSocketMgr::onSocketDidWriteData(JOSocket* socket)
{

}

void JOSocketMgr::onSocketWithError(JOSocket* socket, JOSocketError error)
{

}


NS_JOFW_END