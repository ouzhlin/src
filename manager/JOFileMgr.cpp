/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOFileMgr.cpp
AUTHOR:  James Ou 
*********************************************************************/

//#include "zlib/zlib.h"
#include <zlib.h>
#include "manager/JOFileMgr.h"
#include "manager/vo/JOFileCacheVO.h"
#include "manager/JOCachePoolMgr.h"
#include "manager/JOTickMgr.h"

#include "utils/JOTime.h"
#include "utils/JOLog.h"
#include "utils/JOMd5.h"
#include "utils/JOPath.h"

#include <algorithm>
#include <ios>
#include <fstream>

#if _WIN32
#include <Windows.h>
#include <ShlObj.h>
//#include <regex>
#include <io.h>
#include <direct.h>
#define ACCESS _access 
#define DEST_DELIMITER '\\'
#define TEMP_DELIMITER '/'
#define MAKE_DIR(strPath) _mkdir(strPath)
#define REMOVE_DIR(strPath) _rmdir(strPath)
#else
#include <stdio.h>
#include <stdarg.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define ACCESS access
#define DEST_DELIMITER '/'
#define TEMP_DELIMITER '\\'
#define MAKE_DIR(strPath) JOFileMgr::_makeDir(strPath);
#define REMOVE_DIR(strPath) rmdir(strPath)
#endif

using namespace std;

NS_JOFW_BEGIN

/*
 //////////////////////////
  vector<char>和char *的转换
 //////////////////////////
 
 vector<char> vChar;
 vChar.push_back('a');
 vChar.push_back('b');
 ....
 char *str = new char[vChar.size()+1];
 copy(vChar.begin(),vChar.end(), str);
 str[vChar.size()]=0;
 cout<<str<<endl;
 delete []str;
 */
#define segment_size 1460//largest tcp data segment
/*
//返回的值需要free
*/
char * JOFileMgr::ungzip(std::vector<char> *source, int *len)
{
    std::string str(source->begin(), source->end());
    int ret,have;
    int offset=0;
    z_stream d_stream;
    //Byte compr[segment_size]={0};
    Byte uncompr[segment_size*4]={0};
    //memcpy(compr,(Byte*)str.c_str(),source->size());
    
    //delete []str;
    
    uLong comprLen, uncomprLen;

	/*
	一开始写成了comprlen=sizeof(compr)以及comprlen=strlen(compr)，后来发现都不对。
	sizeof(compr)永远都是segment_size，显然不对，strlen(compr)也是不对的，因为strlen只算到\0之前，
	但是gzip或者zlib数据里\0很多。
	*/
    comprLen = source->size();
    uncomprLen = segment_size*4;
    strcpy((char*)uncompr, "garbage");
    d_stream.zalloc = Z_NULL;
    d_stream.zfree = Z_NULL;
    d_stream.opaque = Z_NULL;
	/*
	inflateInit和inflateInit2都必须初始化next_in和avail_in
	*/
    d_stream.next_in = Z_NULL;
	/*
	deflateInit和deflateInit2则不用
	*/
    d_stream.avail_in = 0;
    ret = inflateInit2(&d_stream,47);
    if(ret!=Z_OK)
    {
        printf("inflateInit2 error:%d",ret);
        return NULL;
    }
    d_stream.next_in=(Byte*)str.c_str();
    d_stream.avail_in=comprLen;
    
    int chunkSize = 2048;
    int sizePlus = 1;
    //char *des = new char[chunkSize * sizePlus];
    char *des = (char*)malloc(chunkSize * sizePlus);
    do
    {
        d_stream.next_out=uncompr;
        d_stream.avail_out=uncomprLen;
        ret = inflate(&d_stream,Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR) {
            return nullptr;
        }
        //assert(ret != Z_STREAM_ERROR);
        switch (ret)
        {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&d_stream);
                return NULL;
        }
        have=uncomprLen-d_stream.avail_out;
        bool bChange = false;
        while (offset+uncomprLen>=chunkSize*sizePlus)
        {
            ++sizePlus;
            bChange = true;
        }
        if (bChange == true){
            char* oldBuffer = des;
            des = (char*)malloc(chunkSize * sizePlus);
            memcpy(des,oldBuffer,offset);
            free(oldBuffer);
        }
		/*
		这里一开始我写成了memcpy(des+offset,d_stream.next_out,have);
		//后来发现这是不对的，因为next_out指向的下次的输出，现在指向的是无有意义数据的内存。见下图
		*/
        memcpy(des+offset,uncompr,have);
        offset+=have;
    }while(d_stream.avail_out==0);
    inflateEnd(&d_stream);
    memcpy(des+offset,"\0",1);
    *len = offset+1;
    return des;
}
/*
//把一个字节流用zip算法压缩，压缩结果存放在result_buffer里面
//src_buffer：    [IN]输入buffer的起始地址
//src_len:        [IN]输入buffer长度
//result_buffer:[OUT]存放压缩结果
//返回值：true-成功，buffer存放压缩结果
//         false-压缩失败
*/
bool JOFileMgr::zip_buffer(const char *src_buffer, const unsigned int src_len,
                std::vector<char> &result_buffer)
{
    if ((NULL == src_buffer) || (0 == src_len))
    {
        return false;
    }
	/*
	//得到要保存压缩结果的最小buffer大小
	*/    
    unsigned long min_length = compressBound(src_len);
    result_buffer.resize(min_length);
    if (Z_OK != compress((Bytef*)&result_buffer[0],
                         &min_length, (Bytef*)src_buffer, src_len))
    {
		/*
		压缩失败
		*/
        return false;
    }
    result_buffer.resize(min_length);
    
    return true;
}

/*
辅助函数，用来得到解压缩的时候所使用的临时缓存的大小，这里我只是尽量取得一个合理的值.
具体怎么获得可以修改
*/
unsigned int JOFileMgr::get_temp_buffer_size(const unsigned int src_length)
{
    const int MB = 1024*1024;
    if (src_length < 1*MB)
    {
        return 1*MB;
    }
    else if ((src_length >= 1*MB) && (src_length < 8*MB))
    {
        return 8*MB;
    }
    else
    {
        return 16*MB;
    }
}
/*

//从字节流中把数据解压出来
//src_buffer:        [in]待解压的buffer起始地址
//src_len:            [in]带解压buffer长度
//result_buffer:    [out]解压结果
//返回值：true-解压成功，buffer存放解压结果
//          false-解压失败
*/
bool JOFileMgr::unzip_buffer(const char *src_buffer, const unsigned int src_len,
                  std::vector<char> &result_buffer)
{
    if ((NULL == src_buffer) || (0 == src_len))
    {
        return false;
    }
    
	/*
	提供临时缓存，保存解压的中间结果
	*/
    const unsigned int temp_buffer_size = get_temp_buffer_size(src_len);
    std::vector<char> temp_buffer(temp_buffer_size);
	/*
	使用默认值
	*/
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.next_in = Z_NULL;
    zs.avail_in = 0;
    zs.next_out = Z_NULL;
    zs.avail_out = 0;
    zs.opaque = NULL;
    if (Z_OK != inflateInit(&zs))
    {
        return false;
    }
    zs.next_in = (Bytef*)src_buffer;
    zs.avail_in = src_len;
    zs.next_out = (Bytef*)&temp_buffer[0];
    zs.avail_out = temp_buffer_size;
    bool decompress_result = true;
    for (;;)
    {
        int ret_val = inflate(&zs, Z_SYNC_FLUSH);
        if ((Z_OK != ret_val) && (Z_STREAM_END != ret_val))
        {
			/*
			发生错误
			*/
            decompress_result = false;
            break;
        }
		/*
		得到缓存里面有效数据长度
		*/
        unsigned int valid_data_size = temp_buffer_size - zs.avail_out;
        result_buffer.insert(result_buffer.end(), 
                             &temp_buffer[0], &temp_buffer[0]+valid_data_size);
        
		/*
		待解压数据没有处理完
		每次把输出缓冲区重置，免得每次都去计算输出缓冲区应该开始的地址
		*/
        if (Z_OK == ret_val)
        {
            zs.next_out = (Bytef*)&temp_buffer[0];
            zs.avail_out = temp_buffer_size;    
        }
		/*
		所有数据处理完成
		*/
        else if (Z_STREAM_END == ret_val)
        {
            break;
        }
    }
    inflateEnd(&zs);
    
    return decompress_result;
}

#if _WIN32

void JOFileMgr::_getFilePathInDIR( const char* dirPath, FILELIST &fileList, const char* suffix/*=""*/ )
{
	std::string searchPath = JOPath::standardisePath(dirPath, true);
	std::string tmpPath = searchPath;
	tmpPath.append("*.*");

	long lf;
	_finddata_t file;
	/*
	_findfirst返回的是long型; long __cdecl _findfirst(const char *, struct _finddata_t *)
	*/
	if ((lf = _findfirst(tmpPath.c_str(), &file)) == -1l)
		return;

	
	std::string path = searchPath;
	if (strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0 && _haveSuffix(file.name, suffix))
	{		
		path.append(file.name);
		
		if ((file.attrib&_A_SUBDIR) == _A_SUBDIR){
			_getFilePathInDIR(path.c_str(), fileList, suffix);			
		}
		else{
			fileList.push_back(path);
		}		
	}	
	while(_findnext(lf, &file) == 0)
	{		
		if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..")==0 || !_haveSuffix(file.name, suffix)){
			continue;
		}
		path = searchPath;
		path.append(file.name);
		if ((file.attrib&_A_SUBDIR) == _A_SUBDIR){
			JOFileMgr::_getFilePathInDIR(path.c_str(), fileList, suffix);
			continue;
		}
		fileList.push_back(path);		
	}
	_findclose(lf);
}

#else

void JOFileMgr::_makeDir(const char *strPath)
{
	DIR *dir = NULL;	 
	dir = opendir( strPath );
	if( dir ) {
		closedir(dir);
		return;
	}
	if (!mkdir(strPath, 0777))
	{
		LOG_DEBUG("JOFileMgr", "fail to create dir %s", strPath);
		LOG_DEBUG("JOFileMgr", "mkdir error: %s\n", strerror(errno));
	}
}


int JOFileMgr::testdir(char *path)
{
	struct stat buf;
	if(lstat(path,&buf)<0)
	{
		return 0;
	}
	if(S_ISDIR(buf.st_mode))
	{
		return 1; //directory
	}
	return 0;
}

void JOFileMgr::_getFilePathInDIR( const char* dirPath, FILELIST &fileList, const char* suffix/*=""*/ )
{
	std::string searchPath = JOPath::standardisePath(dirPath, true);
	DIR *db;
	struct dirent *p;
	db=opendir(searchPath.c_str());
	if(db==NULL) return;
	const std::string strSuffix = suffix;
	while ((p=readdir(db)))
	{
		if((strcmp(p->d_name,".")!=0)&&(strcmp(p->d_name,"..")!=0))
		{
			char filename[256];
			memset(filename,0,256);
			sprintf(filename,"%s%s",searchPath.c_str(), p->d_name);
			if(JOFileMgr::testdir(filename)){
				JOFileMgr::_getFilePathInDIR(filename, fileList, strSuffix.c_str());
			}
			else if (_haveSuffix(p->d_name, strSuffix.c_str()))	{
				fileList.push_back(filename);				
			}
		}
	}
	closedir(db);
}

#endif
/*
#include <vector>
#include "zlib.h"

//把一个字节流用zip算法压缩，压缩结果存放在result_buffer里面
//src_buffer：    [IN]输入buffer的起始地址
//src_len:        [IN]输入buffer长度
//result_buffer:[OUT]存放压缩结果
//返回值：true-成功，buffer存放压缩结果
//         false-压缩失败
bool zip_buffer(const char *src_buffer, const unsigned int src_len,
std::vector<char> &result_buffer)
{
if ((NULL == src_buffer) || (0 == src_len))
{
return false;
}
//得到要保存压缩结果的最小buffer大小
unsigned long min_length = compressBound(src_len);
result_buffer.resize(min_length);
if (Z_OK != compress((Bytef*)&result_buffer[0],
&min_length, (Bytef*)src_buffer, src_len))
{//压缩失败
return false;
}
result_buffer.resize(min_length);

return true;
}

//辅助函数，用来得到解压缩的时候所使用的临时缓存的大小，这里我只是尽量取得一个合理的值.
//具体怎么获得可以修改
unsigned int get_temp_buffer_size(const unsigned int src_length)
{
const int MB = 1024*1024;
if (src_length < 1*MB)
{
return 1*MB;
}
else if ((src_length >= 1*MB) && (src_length < 8*MB))
{
return 8*MB;
}
else
{
return 16*MB;
}
}

//从字节流中把数据解压出来
//src_buffer:        [in]待解压的buffer起始地址
//src_len:            [in]带解压buffer长度
//result_buffer:    [out]解压结果
//返回值：true-解压成功，buffer存放解压结果
//          false-解压失败
bool unzip_buffer(const char *src_buffer, const unsigned int src_len,
std::vector<char> &result_buffer)
{
if ((NULL == src_buffer) || (0 == src_len))
{
return false;
}

//提供临时缓存，保存解压的中间结果
const unsigned int temp_buffer_size = get_temp_buffer_size(src_len);
std::vector<char> temp_buffer(temp_buffer_size);
//使用默认值
z_stream zs;
zs.zalloc = Z_NULL;
zs.zfree = Z_NULL;
zs.next_in = (Bytef*)src_buffer;
zs.avail_in = src_len;
zs.next_out = (Bytef*)&temp_buffer[0];
zs.avail_out = temp_buffer_size;
zs.opaque = NULL;
if (Z_OK != inflateInit(&zs))
{
return false;
}

bool decompress_result = true;
for (;;)
{
int ret_val = inflate(&zs, Z_SYNC_FLUSH);
if ((Z_OK != ret_val) && (Z_STREAM_END != ret_val))
{
//发生错误
decompress_result = false;
break;
}
//得到缓存里面有效数据长度
unsigned int valid_data_size = temp_buffer_size - zs.avail_out;
result_buffer.insert(result_buffer.end(),
&temp_buffer[0], &temp_buffer[0]+valid_data_size);

if (Z_OK == ret_val)
{//待解压数据没有处理完
//每次把输出缓冲区重置，免得每次都去计算输出缓冲区应该开始的地址
zs.next_out = (Bytef*)&temp_buffer[0];
zs.avail_out = temp_buffer_size;
}
else if (Z_STREAM_END == ret_val)
{//所有数据处理完成
break;
}
}
inflateEnd(&zs);

return decompress_result;
}
*/

std::function<std::string(const std::string&)> JOFileMgr::s_fileData2Md5Fun = nullptr;
std::function<JOData*(const std::string&)> JOFileMgr::s_getFileDataCall = nullptr;
std::function<bool(const std::string&)> JOFileMgr::s_isFileExistCall = nullptr;

JOFileMgr::JOFileMgr()
: m_checkInterval(180)
, m_removeInterval(160)
{
}

JOFileMgr::~JOFileMgr()
{
	clearFileCache();
}


bool JOFileMgr::MkMulDir(const std::string& pPath)
{
	if (pPath == "")
		return false;
	std::string strPath(pPath);
	std::replace(strPath.begin(), strPath.end(), TEMP_DELIMITER, DEST_DELIMITER);
	if (/*ACCESS(strPath.c_str(), 0) != -1 ||*/ strPath.empty())
		return true;
	std::string dirPath = strPath.substr(0, strPath.rfind(DEST_DELIMITER));
	if (!dirPath.empty() && dirPath != pPath)
	{
		MkMulDir((dirPath).c_str());	
	}
	
	MAKE_DIR(strPath.c_str());
	return true;
}


bool JOFileMgr::copyData( unsigned char* resData, unsigned int dataLen, const char* destPath )
{	
	if (dataLen> 0)
	{
		std::string outBasename;
		std::string outPath;
		JOPath::splitFilename(destPath, outBasename, outPath);
		JOFileMgr::MkMulDir(outPath.c_str());

		FILE *fp = fopen(destPath,"wb");
		if (fp)
		{
			size_t write = fwrite(resData, 1, dataLen, fp);
            if (write <= 0) {
                LOG_WARN("JOFileMgr", "copy to %s fail", destPath);
            }
			fflush(fp);
			fclose(fp);
			return true;
		}
	}
	
	return false;
}

bool JOFileMgr::copyFile( const char* srcPath, const char* destPath )
{
   
    FILE *fp = fopen(srcPath, "rb");
    if (fp){
        unsigned char* buffer = nullptr;
        size_t size = 0;
        size_t readsize;

        fseek(fp,0,SEEK_END);
        size = ftell(fp);
        fseek(fp,0,SEEK_SET);
    
        buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
    
        
        readsize = fread(buffer, sizeof(unsigned char), size, fp);
        fclose(fp);
        
        if (readsize < size)
        {
            buffer[readsize] = '\0';
        }
        if (nullptr == buffer || 0 == readsize || readsize != size)
        {
            LOG_WARN("JOFileMgr", "copy [%s] fail", srcPath);
        }
        else
        {
			std::string outBasename;
			std::string outPath;
			JOPath::splitFilename(destPath, outBasename, outPath);
			JOFileMgr::MkMulDir(outPath.c_str());

            FILE *fwp = fopen(destPath,"wb");
            if (fwp)
            {
                size_t write = fwrite(buffer, 1, readsize, fp);
                if (write <= 0) {
                    LOG_WARN("JOFileMgr", "copy [%s] fail", srcPath);
                }
                fflush(fwp);
                fclose(fwp);
                return true;
            }
        }
    }
    return false;
}


bool JOFileMgr::moveFile( const char* resPath, const char* destPath )
{
	rename(resPath, destPath);
	return true;
}

bool JOFileMgr::removeFile( const char* filePath )
{
	remove(filePath);
	REMOVE_DIR(filePath);
	return true;
}

/************************************************************************/
/*getFilePathInDIR相关*/
/************************************************************************/
bool JOFileMgr::_haveSuffix( const char* fileName, const char* suffix )
{	
	if (strlen(suffix)<1)
	{
		return true;
	}
	std::string strPath = fileName;
	/*
	先创建文件夹
	*/
	size_t pos = strPath.rfind(".");
	if (-1 != pos)
	{		
		strPath = strPath.substr(pos+1, strPath.length());

		if (strPath.compare(suffix) == 0)
		{
			return true;
		}
	}
	return false;
}

void JOFileMgr::getFilePathInDIR( const char* dirPath, std::list<std::string> &fileList, const char* suffix/*=""*/ )
{
	JOFileMgr::_getFilePathInDIR(dirPath, fileList, suffix);
}


bool JOFileMgr::removeAllFileInPath( const char* path )
{
	std::list<std::string> fileList;
	JOFileMgr::_getFilePathInDIR(path, fileList);
	std::list<std::string>::iterator itr = fileList.begin();
	while (itr != fileList.end())
	{
		removeFile((*itr).c_str());
		++itr;
	}
	return true;
}

bool JOFileMgr::wirteFile(const char* srcPath, const unsigned char* data, const unsigned int dataLen)
{
	FILE *fp = fopen(srcPath, "wb");
	if (fp){
		std::string outBasename;
		std::string outPath;
		JOPath::splitFilename(srcPath, outBasename, outPath);
		JOFileMgr::MkMulDir(outPath.c_str());
		size_t write = fwrite(data, 1, dataLen, fp);
		if (write <= 0) {
			LOG_WARN("JOFileMgr", "copy [%s] fail", srcPath);
		}
		fflush(fp);
		fclose(fp);
		return true;
	}
	return false;
}

/*
获取文件的Md5
*/
void JOFileMgr::setFile2Md5Function(const std::function<std::string(const std::string&)> fileData2Md5Fun)
{
    s_fileData2Md5Fun = fileData2Md5Fun;
}
std::string JOFileMgr::fileMd5(const std::string& filePath)
{
    if (s_fileData2Md5Fun) {
        return s_fileData2Md5Fun(filePath);
    }
    return "";
}

std::string JOFileMgr::stringMd5(const std::string& str)
{
	JOMd5 md5;
    md5.update(str);
    return md5.toString();
}

/*
std::string JOFileMgr::getFileExplorerDir(std::string rootPath)
{
#if _WIN32
	LPITEMIDLIST   pList = NULL;
	BROWSEINFOA bs;
	char buff[MAX_PATH] = "\0";
	memset(&bs, 0, sizeof(BROWSEINFOA));
	bs.pszDisplayName = buff;
	bs.pidlRoot = pList;
	bs.lpszTitle = "select dir";
	bs.ulFlags = BIF_RETURNONLYFSDIRS;
	bs.lParam = NULL;
	bs.lpfn = NULL;
	bs.hwndOwner = NULL;
	if (rootPath.length() > 0)
	{
		LPITEMIDLIST  pIdl = NULL;
		IShellFolder* pDesktopFolder;
		char          szPath[MAX_PATH];
		OLECHAR       olePath[MAX_PATH];
		ULONG         chEaten;
		ULONG         dwAttributes;

		strcpy(szPath, rootPath.c_str());
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPath, -1, olePath, MAX_PATH);
			pDesktopFolder->ParseDisplayName(NULL, NULL, olePath, &chEaten, &pIdl, &dwAttributes);
			pDesktopFolder->Release();
		}
		bs.pidlRoot = pIdl;
	}
	pList = SHBrowseForFolderA(&bs);
	if (pList){
		if (SHGetPathFromIDListA(pList, buff))
		{
			return JOPath::standardisePath(buff, false);
		}
	}
#endif
	return "";
}
*/
void JOFileMgr::setGetFileDataCall(const std::function<JOData*(const std::string&)> call)
{
	s_getFileDataCall = call;
}


//////////////////////////////////////////////////////////////////////////
JOData* JOFileMgr::getFileData(const std::string& fPath)
{
	std::unordered_map<std::string, JOFileCacheVO*>::iterator itr = m_fileDataMap.find(fPath);
	if (itr != m_fileDataMap.end()){
		itr->second->m_useTime = JOTime::getTimeofday();
		return itr->second->m_fileData;
	}
	if (s_getFileDataCall){		
		JOData* d = s_getFileDataCall(fPath);
		if (d){
			JOFileCacheVO* vo = POOL_GET(JOFileCacheVO, "JOFileMgr");
			vo->init(d, JOTime::getTimeofday());
			m_fileDataMap[fPath] = vo;
		}
		else{
			LOG_WARN("JOFileMgr", "get data fail!!![%s]", fPath.c_str());
		}
		return d;
	}
	return nullptr;
}
void JOFileMgr::clearFileCache()
{
	std::unordered_map<std::string, JOFileCacheVO*>::iterator itr = m_fileDataMap.begin();
	while (itr != m_fileDataMap.end()){
		JO_SAFE_DELETE(itr->second->m_fileData);
		POOL_RECOVER(itr->second, JOFileCacheVO, "JOFileMgr");
		++itr;
	}
	m_fileDataMap.clear();
}

void JOFileMgr::tick()
{
	static float totalInterval = 0;
	totalInterval += JOTickMgr::Instance()->deltaTime();
	if (totalInterval > m_checkInterval){
		_checkFileCache();
		totalInterval = 0;
	}
}

void JOFileMgr::_checkFileCache()
{
	float curSec = JOTime::getTimeofday();
	JOFileCacheVO* vo = nullptr;
	std::unordered_map<std::string, JOFileCacheVO*>::iterator itr = m_fileDataMap.begin();
	while (itr != m_fileDataMap.end()){
		vo = itr->second;
		if ((curSec - vo->m_useTime)>m_removeInterval){
			JO_SAFE_DELETE(vo->m_fileData);
			POOL_RECOVER(vo, JOFileCacheVO, "JOFileMgr");
			itr = m_fileDataMap.erase(itr);
		}
		else{
			++itr;
		}		
	}
}

bool JOFileMgr::isFileExist(const std::string& pPath)
{
	if (s_isFileExistCall)
	{
		return s_isFileExistCall(pPath);
	}
	return false;
}

void JOFileMgr::setFileExistCall(const std::function<bool(const std::string&)> call)
{
	s_isFileExistCall = call;
}





NS_JOFW_END

