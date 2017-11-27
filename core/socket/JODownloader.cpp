
#include "core/socket/JODownloader.h"
#include "utils/JOPath.h"
#include "utils/JOLog.h"
#include "manager/JOSnMgr.h"
#include "manager/JOTickMgr.h"

#include <thread>

#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "base/CCUserDefault.h"
#include "network/CCDownloader.h"
#include "platform/CCFileUtils.h"

#ifdef MINIZIP_FROM_SYSTEM
#include <minizip/unzip.h>
#else // from our embedded sources
#include "external/unzip/unzip.h"
#endif
#include "cocos2d.h"

NS_JOFW_BEGIN

using namespace std;
using namespace cocos2d;
using namespace cocos2d::network;


#define TEMP_PACKAGE_FILE_NAME          "cocos2dx-update-temp-package.zip"
#define BUFFER_SIZE    8192
#define MAX_FILENAME   512

// Implementation of AssetsManager

JODownloader::JODownloader()
:  _storagePath("")
, _packageUrl("")
, _downloader(new Downloader())
, _connectionTimeout(0)
, _isDownloading(false)
, _oldeReplacePath("")
,_newReplacePath("") 
,_md5String("")
{
	m_sn = JOSnMgr::Instance()->getSn();
    // convert downloader error code to AssetsManager::ErrorCode
    _downloader->onTaskError = [this](const DownloadTask& task,
                                      int errorCode,
                                      int errorCodeInternal,
                                      const std::string& errorStr)
    {
        _isDownloading = false;
        
		if (nullptr == this->errorCall)
        {
            return;
        }
        auto err = (DownloadTask::ERROR_FILE_OP_FAILED == errorCode) ? ErrorCode::CREATE_FILE : ErrorCode::NETWORK;
		this->errorCall(_packageUrl, (int)err);
    };
    
    // progress callback
    _downloader->onTaskProgress = [this](const DownloadTask& task,
                                         int64_t bytesReceived,
                                         int64_t totalBytesReceived,
                                         int64_t totalBytesExpected)
    {
        if (nullptr == this->progressCall)
        {
            return;
        }
        
        int percent = totalBytesExpected ? int(totalBytesReceived * 100 / totalBytesExpected) : 0;
		this->progressCall(_packageUrl, percent);
		
        //CCLOG("downloading... %d%%", percent);
    };
    
    // get version from version file when get data success
    _downloader->onDataTaskSuccess = [this](const DownloadTask& task,
                                            std::vector<unsigned char>& data)
    {
		if (FileUtils::getInstance()->_isEqualMd5Callback && _md5String.empty() > 0)
		{
			if (!FileUtils::getInstance()->_isEqualMd5Callback(task.storagePath, _md5String))
			{
				JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
					this->errorCall(_packageUrl, (int)ErrorCode::MD5_ERROR);
				});
				LOG_ERROR("JODownloader", "%s  Md5 error", task.storagePath.c_str());
				_isDownloading = false;
				return;
			}
		}

		std::string suffix = JOPath::getFileSuffix(task.storagePath);
		if (suffix=="zip")
		{
			downloadAndUncompress(task.storagePath);
		}
		else{
			if (this->successCall){
				this->successCall(_packageUrl);
			}
			_isDownloading = false;
		}
    };
    
    // after download package, do uncompress operation
    _downloader->onFileTaskSuccess = [this](const DownloadTask& task)
    {
		this->_onHandleDownloadSuccess(task.storagePath);
    };
}
void JODownloader::_onHandleDownloadSuccess(const std::string& filepath)
{
	if (FileUtils::getInstance()->_isEqualMd5Callback && _md5String.empty() == false)
	{

		if (!FileUtils::getInstance()->_isEqualMd5Callback(filepath, _md5String))
		{
			JOTickMgr::Instance()->runInMainThread(m_sn, [&, this]{
				this->errorCall(_packageUrl, (int)ErrorCode::MD5_ERROR);
			});
			LOG_ERROR("JODownloader", "%s  Md5 error", filepath.c_str());
			return;
		}
	}

	std::string suffix = JOPath::getFileSuffix(filepath);
	if (suffix == "zip")
	{
		downloadAndUncompress(filepath);
	}
	else{
		if (this->successCall){
			this->successCall(_packageUrl);
		}
	}
}




void JODownloader::setCall(ErrorCall errorCallback, ProgressCall progressCallback, SuccessCall successCallback, UnzipProgressCall unzipProgressCall)
{
	this->errorCall = errorCallback;
	this->progressCall = progressCallback;
	this->successCall = successCallback;
	this->unzipProgressCall = unzipProgressCall;
}

JODownloader::~JODownloader()
{
	JOTickMgr::Instance()->cancelInMainRun(m_sn);
	JOSnMgr::Instance()->dispose(m_sn);
    CC_SAFE_DELETE(_downloader);
}

void JODownloader::checkStoragePath()
{
    if (_storagePath.size() > 0 && _storagePath[_storagePath.size() - 1] != '/')
    {
        _storagePath.append("/");
    }
}


#define DOWNLOAD_FILE "downloadFile.temp";
bool JODownloader::checkUpdate()
{
	// add by James
	if ( _isDownloading){
		LOG_WARN("JODownloader", "now is downloading, please try later");
		return false;
	}
	if (_packageUrl.size() == 0 ){
		LOG_WARN("JODownloader", "no url !!!!!!");
		return false;
	}
	// start download;
	std::string filename = JOPath::getFileName(_packageUrl);
	if (filename.empty())
	{
		filename = _storagePath + DOWNLOAD_FILE;
	}
	else
	{
		filename = _storagePath + filename;
	}
	_downloader->createDownloadFileTask(_packageUrl, filename);
	_isDownloading = true;
    return true;
}

void sophia_framework::JODownloader::setReplacePath(const char* oldPath, const char* newPath)
{
	JOLockGuard tmpGuard(m_replaceLock);

	_oldeReplacePath = oldPath;
	_newReplacePath = newPath;
	
}

void JODownloader::downloadAndUncompress(const std::string& filepath)
{
	std::thread([this, filepath]()
    {
        do
        {
            // Uncompress zip file.
			if (!uncompress(filepath))
            {
				JOTickMgr::Instance()->runInMainThread(m_sn, [this]{
					if (this->errorCall)
						this->errorCall(this->_packageUrl, (int)ErrorCode::UNCOMPRESS);
				});
                break;
            }
            
			// Delete unloaded zip file.
			if (remove(filepath.c_str()) != 0)
			{
				JOTickMgr::Instance()->runInMainThread(m_sn, [filepath]{
					LOG_WARN("JODownloader", "can not remove downloaded zip file %s", filepath.c_str());
				});
			}

			JOTickMgr::Instance()->runInMainThread(m_sn, [this]{
				// Set resource search path.
				this->setSearchPath();
				if (this->successCall) this->successCall(this->_packageUrl);
			});

        } while (0);
        
        _isDownloading = false;

    }).detach();
}


bool JODownloader::uncompress(const std::string& filepath)
{
    // Open the zip file
	std::string basepath = JOPath::getFileBasePath(filepath);
	string outFileName = filepath;
    unzFile zipfile = unzOpen(FileUtils::getInstance()->getSuitableFOpen(outFileName).c_str());
    if (! zipfile)
    {
		JOTickMgr::Instance()->runInMainThread(m_sn, [outFileName]{
			LOG_WARN("JODownloader", "can not open downloaded zip file %s", outFileName.c_str());
		});
        return false;
    }
    
    // Get info about the zip file
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
    {
		JOTickMgr::Instance()->runInMainThread(m_sn, [outFileName]{
			LOG_WARN("JODownloader", "can not read file global info of %s", outFileName.c_str());
		});
        unzClose(zipfile);
        return false;
    }
    
    // Buffer to hold data read from the zip file
    char readBuffer[BUFFER_SIZE];
    
	JOTickMgr::Instance()->runInMainThread(m_sn, []{
		LOG_DEBUG("JODownloader", "start uncompressing");
	});
	
    
    // Loop to extract all files.
	
	
	unsigned long fileCount = global_info.number_entry;	// add by James
    uLong i;
	for (i = 0; i < fileCount; ++i)
    {
		JOTickMgr::Instance()->runInMainThread(m_sn, [this, i, fileCount]{
			// Set resource search path.
			if (this->unzipProgressCall)
				this->unzipProgressCall(this->_packageUrl, i+1, fileCount);
		});
		
        // Get info about current file.
        unz_file_info fileInfo;
        char fileName[MAX_FILENAME];
        if (unzGetCurrentFileInfo(zipfile,
                                  &fileInfo,
                                  fileName,
                                  MAX_FILENAME,
                                  nullptr,
                                  0,
                                  nullptr,
                                  0) != UNZ_OK)
        {
			JOTickMgr::Instance()->runInMainThread(m_sn, []{
				LOG_WARN("JODownloader", "can not read file info");
			});
            unzClose(zipfile);
            return false;
        }
		// add by James replace the path 
		m_replaceLock.lock();
		if ("" != _oldeReplacePath)
		{
			// only replace first one
			std::string newFileName(fileName);
			string::size_type pos(0);
			if( (pos=newFileName.find(_oldeReplacePath))!=string::npos ) 
			{
				newFileName = newFileName.replace(pos, _oldeReplacePath.length(), _newReplacePath);
			}
			strcpy(fileName, newFileName.c_str());
		}
		m_replaceLock.unlock();

        const string fullPath = basepath + fileName;
        
        // Check if this entry is a directory or a file.
        const size_t filenameLength = strlen(fileName);
        if (fileName[filenameLength-1] == '/')
        {
            // Entry is a directory, so create it.
            // If the directory exists, it will failed silently.
            if (!FileUtils::getInstance()->createDirectory(fullPath))
            {
				JOTickMgr::Instance()->runInMainThread(m_sn, [fullPath]{
					LOG_WARN("JODownloader", "can not create directory %s", fullPath.c_str());
				});
                unzClose(zipfile);
                return false;
            }
        }
        else
        {
            //There are not directory entry in some case.
            //So we need to test whether the file directory exists when uncompressing file entry
            //, if does not exist then create directory
            const string fileNameStr(fileName);
            
            size_t startIndex=0;
            
            size_t index=fileNameStr.find("/",startIndex);
            
            while(index != std::string::npos)
            {
				const string dir = basepath + fileNameStr.substr(0, index);
                
                FILE *out = fopen(FileUtils::getInstance()->getSuitableFOpen(dir).c_str(), "r");
                
                if(!out)
                {
                    if (!FileUtils::getInstance()->createDirectory(dir))
                    {
						JOTickMgr::Instance()->runInMainThread(m_sn, [dir]{
							LOG_WARN("JODownloader", "can not create directory %s", dir.c_str());
						});
                        unzClose(zipfile);
                        return false;
                    }
                }
                else
                {
                    fclose(out);
                }
                
                startIndex=index+1;
                
                index=fileNameStr.find("/",startIndex);
                
            }

            // Entry is a file, so extract it.
            
            // Open current file.
            if (unzOpenCurrentFile(zipfile) != UNZ_OK)
            {
				std::string tmpStr = fileName;
				JOTickMgr::Instance()->runInMainThread(m_sn, [tmpStr]{
					LOG_WARN("JODownloader", "can not open file %s", tmpStr.c_str());
				});
                unzClose(zipfile);
                return false;
            }
            
            // Create a file to store current file.
            FILE *out = fopen(FileUtils::getInstance()->getSuitableFOpen(fullPath).c_str(), "wb");
            if (! out)
            {
				JOTickMgr::Instance()->runInMainThread(m_sn, [fullPath]{
					LOG_WARN("JODownloader", "can not open destination file %s", fullPath.c_str());
				});
                unzCloseCurrentFile(zipfile);
                unzClose(zipfile);
                return false;
            }
            
            // Write current file content to destinate file.
            int error = UNZ_OK;
            do
            {
                error = unzReadCurrentFile(zipfile, readBuffer, BUFFER_SIZE);
                if (error < 0)
                {
					std::string tmpStr = fileName;
					JOTickMgr::Instance()->runInMainThread(m_sn, [tmpStr, error]{
						LOG_WARN("JODownloader", "can not read zip file %s, error code is %d", tmpStr.c_str(), error);
					});
                    unzCloseCurrentFile(zipfile);
                    unzClose(zipfile);
                    return false;
                }
                
                if (error > 0)
                {
                    fwrite(readBuffer, error, 1, out);
                }
            } while(error > 0);
            
            fclose(out);
        }
        
        unzCloseCurrentFile(zipfile);
        
        // Goto next entry listed in the zip file.
		if ((i + 1) < fileCount)
        {
            if (unzGoToNextFile(zipfile) != UNZ_OK)
            {
				JOTickMgr::Instance()->runInMainThread(m_sn, []{
					LOG_WARN("JODownloader", "can not read next file");
				});
                unzClose(zipfile);
                return false;
            }
        }
    }
	JOTickMgr::Instance()->runInMainThread(m_sn, []{
		LOG_DEBUG("JODownloader", "end uncompressing");
	});
    unzClose(zipfile);
    
    return true;
}

void JODownloader::setSearchPath()
{
    vector<string> searchPaths = FileUtils::getInstance()->getSearchPaths();
    vector<string>::iterator iter = searchPaths.begin();
    searchPaths.insert(iter, _storagePath);
    FileUtils::getInstance()->setSearchPaths(searchPaths);
}

void JODownloader::setStoragePath(const char *storagePath)
{
    _storagePath = storagePath;
    checkStoragePath();
}



NS_JOFW_END
