/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOCommandQueue.cpp
AUTHOR:  James Ou 
*********************************************************************/

#include "manager/commandqueue/JOCommandQueue.h"
#include "manager/commandqueue/JOCommandVO.h"
#include "manager/JOCachePoolMgr.h"

NS_JOFW_BEGIN

JOCommandQueue::JOCommandQueue() 
:m_bAsyn(false)
{
}

JOCommandQueue::~JOCommandQueue()
{
	clearAll();
	_clearRemoveList();
}

void JOCommandQueue::update()
{
	_clearRemoveList();	

	if (!m_vCommands.empty())
	{
        JOLockJudgeGuard(m_lock, !m_bAsyn);
		JOCommandVO *pCommand = *(m_vCommands.begin());
		if (pCommand && pCommand->run() == true) {
			m_removeList.push_back(pCommand);
			m_vCommands.remove(pCommand);
		}
	}
	    
}

bool JOCommandQueue::removeCommand(unsigned int sn)
{
	JOLockJudgeGuard(m_lock, !m_bAsyn);

	std::list<JOCommandVO*>::iterator itr = m_vCommands.begin();
        while (itr != m_vCommands.end()) {
			JOCommandVO* vo = (*itr);
			if (vo->isId(sn)) {
				vo->setCancel(true);
				m_removeList.push_back(vo);
				m_vCommands.remove(vo);
                return true;
            }
            ++itr;
        }
	return false;
}

void JOCommandQueue::clearAll()
{
	JOLockJudgeGuard(m_lock, !m_bAsyn);
    std::list<JOCommandVO*>::iterator itr = m_vCommands.begin();
    while (itr != m_vCommands.end())
    {
        m_removeList.push_back(*itr);
        ++itr;
    }
    m_vCommands.clear();
}

void JOCommandQueue::pushBack(JOCommandVO* pCommand)
{
	JOLockJudgeGuard(m_lock, !m_bAsyn);
    m_vCommands.push_back(pCommand);
}
void JOCommandQueue::pushFront(JOCommandVO* pCommand)
{
	JOLockJudgeGuard(m_lock, !m_bAsyn);
    m_vCommands.push_front(pCommand);
}

void JOCommandQueue::_clearRemoveList()
{
	std::list<JOCommandVO*>::iterator itr = m_removeList.begin();
	while (itr != m_removeList.end())
	{
		POOL_RECOVER(*itr, JOCommandVO, "JOCommandQueue");
		++itr;
	}
	m_removeList.clear();
}

void JOCommandQueue::setAsynchronousMode(bool beAsyn)
{
	m_bAsyn = beAsyn;
}


NS_JOFW_END

