/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOCacheMgr.cpp
AUTHOR:  James Ou 
*********************************************************************/
#include "manager/JOTaskMgr.h"
#include "manager/JOSnMgr.h"
#include "utils/JOLuaUtils.h"
/*
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
*/
//#include <pthread.h>

NS_JOFW_BEGIN

/*
主线程
*/
static std::mutex s_mainTaskQueueMutex;

/*
子线程
*/
static std::mutex s_asynTaskQueueMutex;

/*
条件变量
*/
static std::mutex s_SleepMutex;
static std::condition_variable s_SleepCondition;

/*
线程id
*/
static std::thread s_workThread;

/*
任务循环标记
*/
static bool need_quit = false;
static bool asynInited = false;

JOITask::JOITask():_bAvailable(false),_taskHandle(nullptr)
{
    
}
JOITask::JOITask(const std::function<void(void)> &taskHandle):_bAvailable(true)
{
    _taskHandle = taskHandle;
}
void JOITask::run()
{
    if (_taskHandle) {
        _taskHandle();
    }
}

bool JOITask::isAvailable()
{
    return _bAvailable;
}

JOLuaTask::JOLuaTask(LUA_FUNCTION luaFun, const void* luaState):_luaFun(luaFun),_luaState(luaState)
{
    _bAvailable = true;
}

void JOLuaTask::run()
{
    if (_luaState) {
        JOLuaUtils::executeFunctionInThread(_luaFun, _luaState);
        return;
    }
    std::list<JOLuaArgs> argList;
    JOLuaUtils::executeFunction(_luaFun, argList);
}


JOTaskMgr::JOTaskMgr() :_sn(-1)
{
	_sn = JOSnMgr::Instance()->getSn();
}

JOTaskMgr::~JOTaskMgr()
{
	JOSnMgr::Instance()->dispose(_sn);
	/*
	子线程析构 BEGIN
	*/
    need_quit = true;
	/*
	销毁任务队列
	唤醒线程，清理资源
	*/
    clearAsynTask();

    s_SleepCondition.notify_all();
    if (s_workThread.joinable()) {
        s_workThread.join();
    }
	/*
	子线程析构 END
	*/
    
	/*
	主线程析构 BEGIN
	销毁互斥锁
	*/
    clearMainTask();
	/*
	主线程析构 END
	*/    
}

/*
工作线程
*/
static void* workThread(void *data){
    while (true)
    {
		/*
		如果接收到退出信号，退出循环
		*/
        if (need_quit)
        {
            break;
        }
        
		/*
		从任务队列中获取在队头的任务
		*/
		JOITask *task = JOTaskMgr::Instance()->popAsynTask();
        
		/*
		如果队列中没有任务
		线程进行睡眠状态，等待主线程添加任务并唤醒当前线程
		*/
        if (task == nullptr)
        {            
            std::unique_lock<std::mutex> lk(s_SleepMutex);
            s_SleepCondition.wait_for(lk,std::chrono::milliseconds(1000));
            continue;
        }
        
        s_asynTaskQueueMutex.lock();

        if (task) {
            task->run();
            JOLuaTask* luaT = static_cast<JOLuaTask*>(task);
            if (luaT) {
                delete luaT;
            }
        }

        s_asynTaskQueueMutex.unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
	/*
	如果接收到退出信号，清理任务队列和释放资源
	*/
	JOTaskMgr::Instance()->clearAsynTask();
   /*
    s_SleepCondition.notify_all();
    if (s_workThread.joinable()) {
        s_workThread.join();
    }
    */
    asynInited = false;
    return 0;
}

void JOTaskMgr::_lazyInitAsyn()
{
    if (asynInited == true) {
        return;
    }
    if (s_workThread.joinable()) {
        s_workThread.join();
    }
    s_workThread = std::thread(&workThread, (void*)NULL);
    need_quit = false;
    asynInited = true;
    	
}

void JOTaskMgr::addAsynTask(JOITask *task)
{
    _lazyInitAsyn();
    
	/*
	添加到任务队列
	*/
    s_asynTaskQueueMutex.lock();
    _asynTaskList.push_back(task);
    s_asynTaskQueueMutex.unlock();
    
	/*
	唤醒工作线程
	*/
    s_SleepCondition.notify_one();
}
void JOTaskMgr::addMainTask(JOITask *task)
{
	/*
	添加到任务队列
	*/
    s_mainTaskQueueMutex.lock();
	if (_mainTaskList.size()==0)
	{
		JOTickMgr::Instance()->registerTick(_sn, JO_CBACK_0(JOTaskMgr::tick, this));
	}
    _mainTaskList.push_back(task);
    s_mainTaskQueueMutex.unlock();
	
}

void JOTaskMgr::addAsynLuaTask(LUA_FUNCTION luaFun)
{
    const void *luaState = JOLuaUtils::luaNewThreadState();
    JOLuaTask *luaTask = new JOLuaTask(luaFun, luaState);

    addAsynTask(luaTask);
}
void JOTaskMgr::addMainLuaTask(LUA_FUNCTION luaFun)
{
    JOLuaTask *luaTask = new JOLuaTask(luaFun);
    addMainTask(luaTask);
}

void JOTaskMgr::tick()
{
    JOITask *task = popMainTask();
    s_mainTaskQueueMutex.lock();
    if (task) {
        task->run();
        JOLuaTask* luaT = static_cast<JOLuaTask*>(task);
        if (luaT) {
            delete luaT;
        }
    }
    s_mainTaskQueueMutex.unlock();
}

JOITask* JOTaskMgr::popAsynTask()
{
    JOITask *task = nullptr;
    s_asynTaskQueueMutex.lock();
    if (_asynTaskList.size() > 0)
    {
        task = _asynTaskList.front();
        _asynTaskList.pop_front();
    }
    s_asynTaskQueueMutex.unlock();
    return task;
}
JOITask* JOTaskMgr::popMainTask()
{
    JOITask* task = nullptr;
    s_mainTaskQueueMutex.lock();
    if (_mainTaskList.size() > 0)
    {
        task = _mainTaskList.front();
        _mainTaskList.pop_front();
    }
    if (_mainTaskList.size() == 0)
    {
		JOTickMgr::Instance()->unRegisterTick(_sn);
    }
    s_mainTaskQueueMutex.unlock();
    return task;
}

void JOTaskMgr::clearAsynTask()
{
    TaskList::iterator iter = _asynTaskList.begin();
    s_asynTaskQueueMutex.lock();
    while (iter != _asynTaskList.end()) {
        JOLuaTask* luaT = static_cast<JOLuaTask*>((*iter));
        if (luaT) {
            delete luaT;
        }
        ++iter;
    }
    _asynTaskList.clear();
    s_asynTaskQueueMutex.unlock();
}
void JOTaskMgr::clearMainTask()
{
	JOTickMgr::Instance()->unRegisterTick(_sn);
    s_mainTaskQueueMutex.lock();
    TaskList::iterator iter = _mainTaskList.begin();
    while (iter != _mainTaskList.end()) {
        JOLuaTask* luaT = static_cast<JOLuaTask*>((*iter));
        if (luaT) {
            delete luaT;
        }
        ++iter;
    }
    _mainTaskList.clear();
    s_mainTaskQueueMutex.unlock();
}


NS_JOFW_END

