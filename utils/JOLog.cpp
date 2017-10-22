

#include "utils/JOLog.h"
#include "utils/JOPath.h"
#include <algorithm>
#include <vector>

NS_JOFW_BEGIN

#ifdef WIN32

#include <windows.h>

#else

#include <sstream>
inline std::string to_string(const int& t){
	std::ostringstream os;
	os << t;
	return os.str();
}

inline std::string to_string(const bool& t){
	std::ostringstream os;
	os << t;
	return os.str();
}
inline std::string to_string(const double& t){
	std::ostringstream os;
	os << t;
	return os.str();
}
#endif

//#ifndef JO_LOG
//#define JO_LOG(format, ...) do {} while (0)
//#endif
/*
这里\033[是转义子列，表示后面接的是颜色代码(亮度；背景色；前景色)

01是高亮度，不写是低亮度。40是背景色，40：黑，41：红：42：绿，43：黄，44：青，45：蓝，47：白。
32是前景色：30：黑，31：红，32：绿，33：黄，34：蓝，35：紫，36：青，37：白。
printf("\033[0;40;32m  Begin to download \033[0m \n");
printf("\033[01;40;32m Begin \033[0m\n");
printf("\033[01;40;37m begin \033[0m\n");
*/
std::string JOLog::_logLevel = "";

short JOLog::getLogLevelN(const char* logLevel)
{
	if (_logLevel.compare(logLevel) == 0)
	{
		return 5;
	}
	_logLevel = logLevel;

	if( strcmp(logLevel , "DEBUG") == 0 )
	{
#ifdef WIN32
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);		
		SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN);
#else
        //printf("\33[0;40;33m \n");
#endif
		return 1;
	}
	if( strcmp(logLevel , "INFO") == 0 )
	{
#ifdef WIN32
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);		
		SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_GREEN);
#else
        //printf("\33[0;40;34m \n");
#endif
		return 2;
	}
	if( strcmp(logLevel , "WARN") == 0 )
	{
#ifdef WIN32
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);		
		SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_BLUE);
#else
        //printf("\33[0;40;35m \n");
#endif
		return 3;
	}
	if( strcmp(logLevel , "ERROR") == 0 )
	{
#ifdef WIN32
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);		
		SetConsoleTextAttribute(hOut, FOREGROUND_RED);
#else
        //printf("\33[0;40;32m \n");
#endif
		return 4;
	}
	return 0;
};



const char* JOLog::getLogIgnoreKey(const char* logLevel)
{
	if( strcmp(logLevel , "DEBUG") == 0 )
		return "Log_Ignore_Debug";
	if( strcmp(logLevel , "INFO") == 0 )
		return "Log_Ignore_Info";
	if( strcmp(logLevel , "WARN") == 0 )
		return "Log_Ignore_Warn";
	if( strcmp(logLevel , "ERROR") == 0 )
		return "Log_Ignore_Error";

	return "Log_Ignore_Debug";
};

std::function<void(const char*)> JOLog::_logfun = nullptr;
std::unordered_map<std::string, bool> JOLog::_exclusiveMap;
std::queue<JOLog::LogData> JOLog::logDataQueue;

std::string JOLog::logTitle = "";
bool JOLog::bJOLogShow = true;
short JOLog::gLogLevel = 0;
bool JOLog::beShowInTick = true;
unsigned int JOLog::logCount = 0;

void JOLog::setLogConfig(bool bShow, std::string title, std::string &logLv, bool bInTick)
{
    logTitle=title;
    bJOLogShow=bShow;
    gLogLevel=JOLog::getLogLevelN(logLv.c_str());
    beShowInTick=bInTick;
}

void JOLog::addExclusive(std::string key)
{
    _exclusiveMap[key] = true;
}

void JOLog::setLogFunction(const std::function<void(const char*)> logfun)
{
    _logfun = logfun;
}

void JOLog::tick()
{
    //for(unsigned int i=1; i<10; i++)
    {
        if (logDataQueue.empty()) {
            return;
        }
        LogData log = logDataQueue.front();        
		if (_logfun && getLogLevelN(log.level.c_str()) >= gLogLevel) {			
            _logfun(log.msg.c_str());
        }
        logDataQueue.pop();
    }
}

void JOLog::output( const char* logLevel, const char* logTag, const char* msg )
{
	if( strcmp(logLevel , "ERROR") != 0)
	{
		if (!bJOLogShow){
			return;
		}
		/*
		先看排他项
		*/
		if( !_exclusiveMap.empty() ){
			if( (_exclusiveMap.end() != _exclusiveMap.find(logTag)) ){
				return;
			}
		}
	}

	std::string baseMsg = msg;
	unsigned int msgLen = baseMsg.length();
	while (msgLen>1024)
	{
		_output(logLevel, logTag, baseMsg.substr(0, 1024).c_str());
		msgLen -= 1024;
		baseMsg = baseMsg.substr(1025, msgLen);
	}
	_output(logLevel, logTag, baseMsg.c_str());
}


void JOLog::_output(const char* logLevel, const char* logTag, const char* msg)
{
	if (beShowInTick)
	{
		LogData logD(logLevel, JOString::formatString("%s@%d[%s]%s T(%s)", logTitle.c_str(), logCount++, logLevel, msg, logTag));
		logDataQueue.push(logD);
	}
	else
	{
		if (_logfun && getLogLevelN(logLevel) >= gLogLevel) {
			_logfun(JOString::formatString("%s@%d[%s]%s T(%s)", logTitle.c_str(), logCount++, logLevel, msg, logTag).c_str());
		}
	}
}


std::string JOLog::format_file_name( const char* file )
{
	return JOPath::getFileName(file);
	/*
	std::string f(file);
	std::string::size_type pos = f.find_last_of("/");
	if( pos < 0 )
		pos = f.find_last_of("\\");
	if( pos < 0 )
		return f;
	return f.substr( pos + 1 , f.length() - pos -1 );
	*/
};

std::string JOLog::format_function_name( const char* fun )
{
	std::string f(fun);
	std::string::size_type pos = f.find_last_of(":");
	if( pos < 0 )
		return f;
	else
		return f.substr( pos + 1 , f.length() - pos -1 );
}

NS_JOFW_END
