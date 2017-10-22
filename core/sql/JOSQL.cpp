/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOSQL.cpp
AUTHOR:  James Ou 
*********************************************************************/


#include "core/sql/JOSQL.h"
#include <mutex>
extern "C"
{
#include "core/sql/sqlite3.h"
};
#include "core/sql/JOSqlQuary.h"
#include "utils/JOLuaUtils.h"
#include "utils/JOLog.h"

#include "manager/JOTaskMgr.h"


NS_JOFW_BEGIN

static std::mutex s_asynTaskQueueMutex;

void sqlCallLua(const int luaHandle, JOSqlReader *pReader, const std::string &tableName, bool bFinish)
{
    if (luaHandle != 1) {
        std::list<JOLuaArgs> tempList;
        JOLuaArgs luaArg1;
        luaArg1._argObj = (void*)pReader;
        luaArg1._argObjType = "JOSqlReader";
        luaArg1._argBaseType = JOLuaArgBaseType::eJOLUA_ARGS_TYPE_OBJ;
        tempList.push_back(luaArg1);
        
        JOLuaArgs luaArg2;
        luaArg2._argBase = tableName;
        luaArg2._argBaseType = JOLuaArgBaseType::eJOLUA_ARGS_TYPE_STRING;
        tempList.push_back(luaArg2);

        JOLuaArgs luaArg3;
        luaArg3._argBase = bFinish?"true":"false";
        luaArg3._argBaseType = JOLuaArgBaseType::eJOLUA_ARGS_TYPE_BOOL;
        tempList.push_back(luaArg3);

        JOLuaUtils::executeFunction(luaHandle , tempList);
    }
}

class LuaSqlReadStream : public ISqlReadStreamCallBack
{
public:
    LuaSqlReadStream():luaHandle(-1){}
    virtual void onSqTablelLoad(JOSqlReader *pReader, const std::string &tableName, bool bFinish)
    {
        if (luaHandle != -1)
        {
            sqlCallLua(luaHandle, pReader, tableName, bFinish);
        }
    }
    LUA_FUNCTION luaHandle;
};

static LuaSqlReadStream gDelegate;


sqlite3 *pDB = NULL;
char * errMsg = NULL;
std::string sqlstr;
int result;

JOSQL::JOSQL()
{
    pTask = new JOITask(std::bind(&JOSQL::asynLoadSqlData,this));
}

JOSQL::~JOSQL()
{
    delete pTask;
    pTask = nullptr;
	closeDB();
}

void JOSQL::initDB( const char *db )
{	
	result = sqlite3_open(db, &pDB);  
	if( result != SQLITE_OK ) 
	{
		pDB = nullptr;
		LOG_ERROR("JOSQL", "open db fail, error code:%d reason:%s" , result, errMsg );  
	}
}

void JOSQL::execSql( std::string sql, sqlite3_callback callback /*= nullptr*/, /* Callback function */ void * arg /*= nullptr /* 1st argument to callback */ )
{
	if (pDB == nullptr)
	{
		LOG_ERROR("JOSQL", "pDB == nullptr" ); 
		return;
	}
	try
	{
		result = sqlite3_exec( pDB, sql.c_str(), callback, arg, &errMsg );  
		throw(result);
	}
	catch (int ret)
	{
		if(ret != SQLITE_OK )  
			LOG_ERROR("JOSQL", "exec sql [%s] fail, error code:%d ,reason:%s" ,sql.c_str(), ret, errMsg ); 
	}	
}

int isExisted( void * para, int n_column, char ** column_value, char ** column_name )  
{  
	bool *isExisted_=(bool*)para;  
	*isExisted_=(**column_value)!='0';  
	return 0;  
} 

bool JOSQL::tableIsExist( std::string name )
{
	if (pDB!=nullptr)  
	{  
		bool tableIsExisted;  
		sqlstr = "select count(type) from sqlite_master where type='table' and name ='"+name+"'";  
		result =sqlite3_exec(pDB,sqlstr.c_str(),isExisted,&tableIsExisted,&errMsg);  
		return tableIsExisted;  
	}  
	return false;  
}
  
void JOSQL::createTable( std::string sql, std::string name )
{
	if (!tableIsExist(name))  
	{		
		result = sqlite3_exec(pDB,sql.c_str(),NULL,NULL,&errMsg);  
		if( result != SQLITE_OK )  
			LOG_ERROR("JOSQL", "create table fail, error code:%d reason:%s" , result, errMsg );  
	}  
}

void JOSQL::deleteTable( std::string name )
{
	if (tableIsExist(name))  
	{  
		result = sqlite3_exec(pDB,JOString::formatString("drop table %s",name.c_str()).c_str(),NULL,NULL,&errMsg);  
		if( result != SQLITE_OK )  
			LOG_ERROR("JOSQL", "delete table fail, error code:%d reason:%s" , result, errMsg );  
	}  
}


int loadRecordCount( void * para, int n_column, char ** column_value, char ** column_name )  
{  
	int *count=(int*)para;  
	*count=n_column;  
	return 0;  
}

int JOSQL::getDataCount( std::string sql )
{
	int count=0;  
	sqlite3_exec( pDB, sql.c_str() , loadRecordCount, &count, &errMsg );  
	return count; 
}

int loadRecord( void * para, int n_column, char ** column_value, char ** column_name )  
{  
   LOG_INFO("JOSQL","n_column:%d",n_column);  
      
//    TestVO* testVO = (TestVO*)para;  
      
 //   testVO->mId = atoi(column_value[0]);  
 //   testVO->level = atoi(column_value[1]);  
//    testVO->lastscore = atoi(column_value[2]);  
 //  testVO->bestscore = atoi(column_value[3]);  
 //  testVO->star = atoi(column_value[4]);  
      
      
    /* ??????????????????*/  
    // id level lastscore bestscore star  
//    CCLOG("c[0]:%s,c[1]:%s,c[2]:%s,c[3]:%s,c[4]:%s",column_name[0],column_name[1],column_name[2],column_name[3],column_name[4]);  
//      
    LOG_INFO("JOSQL","id=%s,level=%s,lastscore=%s,bestscore=%s,star=%s",column_value[0],column_value[1],column_value[2],column_value[3],column_value[4]);  
    return 0;  
}

void JOSQL::getDataInfo( std::string sql,void *pSend )
{
	sqlite3_exec( pDB, sql.c_str() , loadRecord, pSend, &errMsg );  
}

void JOSQL::closeDB()
{
	sqlite3_close(pDB);  
}

//std::string sql = "select * from " + std::string(tableName);
//sprintf(sqlBuf, "select * from %s where %s =%d", tableName, keyName, id);
//sprintf(sqlBuf, "select * from %s where %s =\'%s\'", tableName, keyName, key);
//sprintf(sqlBuf, "select * from %s where %s =\'%s\' and %s = %s", tableName, keyName1, key1, keyName2, key2);
void JOSQL::startLoadSqlData( std::string sql,const char* tableName, ISqlReadStreamCallBack* pCallback, bool bAsyn/*=false*/ )
{
	if (tableIsExist(tableName))
	{
        if (bAsyn == false)
        {
            sqlite3_stmt * stat;
            result = sqlite3_prepare( pDB, sql.c_str(), -1, &stat, 0 );
            if (result != SQLITE_OK){
                LOG_ERROR("JOSQL", "exec sql[%s] fail, error code:%d " ,sql.c_str(), result );
                return;
            }
            
            result = sqlite3_step( stat );
            JOSqlReader reader(stat);
            while (result == SQLITE_ROW)
            {
                pCallback->onSqTablelLoad(&reader, tableName, false);
                result = sqlite3_step( stat );
            }
            sqlite3_finalize(stat);
            pCallback->onSqTablelLoad(&reader, tableName, true);
        }
        else
        {
            
            JOLoadSqlInfo info;
            info.sql = sql;
            info.tableName = tableName;
            if (pCallback == &gDelegate)
            {
                info.luaHandle = gDelegate.luaHandle;
            }
            else
            {
                info.pCallback = pCallback;
            }
            
            info.bAvailable = true;
            
            s_asynTaskQueueMutex.lock();
            loadInfoList.push_back(info);
            s_asynTaskQueueMutex.unlock();
            
			JOTaskMgr::Instance()->addAsynTask(pTask);
        }
	}
}

void JOSQL::cancelLoadSqlData(ISqlReadStreamCallBack *pCallback)
{
    s_asynTaskQueueMutex.lock();
    std::list<JOLoadSqlInfo>::iterator itr = loadInfoList.begin();
    while (itr != loadInfoList.end()) {
        if ((*itr).pCallback && (*itr).pCallback == pCallback)
        {
            loadInfoList.erase(itr);
            break;
        }
        ++itr;
    }
    s_asynTaskQueueMutex.unlock();
}

void JOSQL::cancelLoadSqlData(int luaHandle)
{
    s_asynTaskQueueMutex.lock();
    std::list<JOLoadSqlInfo>::iterator itr = loadInfoList.begin();
    while (itr != loadInfoList.end()) {
        if ((*itr).luaHandle && (*itr).luaHandle == luaHandle)
        {
            loadInfoList.erase(itr);
            break;
        }
        ++itr;
    }
    s_asynTaskQueueMutex.unlock();
}

void JOSQL::asynLoadSqlData(void)
{
    s_asynTaskQueueMutex.lock();
    size_t listSize = loadInfoList.size();
    s_asynTaskQueueMutex.unlock();
    
    if (listSize>0) {
        
        s_asynTaskQueueMutex.lock();
        JOLoadSqlInfo info = loadInfoList.front();
        loadInfoList.pop_front();
        s_asynTaskQueueMutex.unlock();
        
        int result;
        do{
            sqlite3_stmt * stat;
            result = sqlite3_prepare( pDB, info.sql.c_str(), -1, &stat, 0 );
            if (result != SQLITE_OK){
                LOG_ERROR("JOSQL", "exec sql[%s] fail, error code:%d " ,info.sql.c_str(), result );
                return;
            }
            
            result = sqlite3_step( stat );
            JOSqlReader reader(stat);
            while (result == SQLITE_ROW)
            {
                if (info.luaHandle!=-1) {
                    sqlCallLua(info.luaHandle, &reader, info.tableName, false);
                }else{
                    info.pCallback->onSqTablelLoad(&reader, info.tableName, false);
                }
                result = sqlite3_step( stat );
            }
            sqlite3_finalize(stat);
            
            if (info.luaHandle!=-1) {
                sqlCallLua(info.luaHandle, &reader, info.tableName, true);
            }else{
                info.pCallback->onSqTablelLoad(&reader, info.tableName, true);
            }
            
            s_asynTaskQueueMutex.lock();
            listSize = loadInfoList.size();
            info = loadInfoList.front();
            loadInfoList.pop_front();
            s_asynTaskQueueMutex.unlock();
            
        }while (listSize>0);
    }
}


void JOSQL::execLuaLoadSqlData( std::string sql,const char* tableName, LUA_FUNCTION luaFunc, bool bAsyn/*=false*/ )
{
	gDelegate.luaHandle = luaFunc;
	startLoadSqlData(sql, tableName, &gDelegate, bAsyn);
}

void JOSQL::tick()
{
    
}

NS_JOFW_END