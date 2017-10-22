#include "core/crypto/JOCrypto.h"

#include <string>
#include <stdlib.h>
#include <list>
#include <stddef.h>

#include "manager/JOFileMgr.h"
#include "utils/JOString.h"
#include "utils/JOPath.h"
#include "utils/JOLog.h"

NS_JOFW_BEGIN

static std::string sm_cKey = "";
static unsigned short sm_keyLen = 0;

static const std::string sm_cSecretKey = "xinker_James";
static const unsigned short sm_secretKeyLen = sm_cSecretKey.length();

static unsigned short sm_headLen = 3;

void JOCrypto::setKey(const char* pKey)
{
	unsigned int tmpLen = strlen(pKey);
	if (tmpLen>4){
		sm_cKey = pKey;
		sm_keyLen = sm_cKey.length();
		return;
	}
	LOG_WARN("JOCrypto", "the key [%s] was not long enough!!!", pKey);
	
	/*
    if(sm_cHead == NULL){
		sm_cHead = (unsigned char *)malloc(sm_keyLen);
    }
    else {
		sm_cHead = (unsigned char *)realloc(sm_cHead, sm_keyLen);
    }
	//混淆密钥
	for (unsigned int i = 0; i<sm_keyLen; i++){
		sm_cHead[i] = sm_cKey[i] ^ sm_cKey[sm_keyLen - 1];
    }
	
	//混淆私人密钥
    if(sm_cSecretHead == NULL){
		unsigned int smKeyLen = sm_cSecretKey.length();
		sm_cSecretHead = (unsigned char*)malloc(smKeyLen);
		for (unsigned int i = 0; i<smKeyLen; i++) {
			sm_cSecretHead[i] = sm_cSecretKey[i] ^ sm_cSecretKey[smKeyLen - 1];
        }
    }
	*/
}


unsigned char * JOCrypto::encode( unsigned char *data, unsigned int data_len, unsigned int *ret_length )
{
	//no key no encode
	if (sm_keyLen < 4){
        *ret_length = data_len;
        return data;
    }
	//allow encode space
	unsigned char* result = (unsigned char *)malloc(data_len + sm_headLen);
    /************************************************************************/
    /*设置密钥包头(取头、尾、中三个位作为包头)
	/*如果文件不足10
	/*(三个位都是取头与密钥进行与或)
    /************************************************************************/

	unsigned int tempKeyLenIdx = 0;
	unsigned int tmpDataIdx = 0;

	if (data_len<10){
		tmpDataIdx = 0;
		while (tmpDataIdx < data_len){
			if (tempKeyLenIdx >= sm_keyLen) {
				tempKeyLenIdx = 0;
			}
			result[sm_headLen + tmpDataIdx] = data[tmpDataIdx] ^ sm_cKey[tempKeyLenIdx];
			++tempKeyLenIdx;
			++tmpDataIdx;
		}
	}
	else{
		unsigned int tmpAllLen = data_len;
		/************************************************************************/
		/* 奇
		/************************************************************************/
		if (data_len & 1){
			tmpAllLen -= 1;
		}
		//start encode
		tmpDataIdx = 0;
		for (tmpDataIdx = 0; tmpDataIdx < tmpAllLen; tmpDataIdx += 2){
			if (tempKeyLenIdx >= sm_keyLen) {
				tempKeyLenIdx = 0;
			}
			result[sm_headLen + tmpDataIdx] = data[tmpDataIdx] ^ sm_cKey[tempKeyLenIdx];
			result[tmpDataIdx + sm_headLen+1] = data[tmpDataIdx + 1];
			++tempKeyLenIdx;
		}
		if (data_len & 1){
			if (tempKeyLenIdx >= sm_keyLen) {
				tempKeyLenIdx = 0;
			}
			result[sm_headLen + tmpAllLen] = (data[tmpAllLen] ^ sm_cKey[tempKeyLenIdx]);
		}
	}

	tmpDataIdx = 0;
	result[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[tmpDataIdx + sm_headLen];
	++tmpDataIdx;
	result[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[(unsigned int)(data_len*0.5f) + sm_headLen];
	++tmpDataIdx;
	result[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[data_len-1 + sm_headLen];
	/*
	tmpDataIdx = 0;
	while (tmpDataIdx < sm_headLen){
		result[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[tmpDataIdx + sm_headLen];
		++tmpDataIdx;
	}
	*/
	*ret_length = data_len + sm_headLen;
    return result;
}

unsigned char * JOCrypto::decode( unsigned char *data, unsigned int data_len, unsigned int *ret_length, bool orgReturn )
{
	//no key no decode
	if (sm_keyLen < 4 || data_len<=sm_headLen){
		if (orgReturn == true){
			*ret_length = data_len;
			return data;
		}
		return nullptr;
    }

	unsigned char *result = data;
	unsigned int tempKeyLenIdx = 0;
	unsigned int tmpDataIdx = 0;
	unsigned int tmpDataLen = data_len - sm_headLen;
	unsigned char tmpHeadBuf[3];

	
	tmpHeadBuf[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[tmpDataIdx + sm_headLen];
	++tmpDataIdx;
	tmpHeadBuf[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[(unsigned int)(tmpDataLen*0.5f) + sm_headLen];
	++tmpDataIdx;
	tmpHeadBuf[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[data_len - 1];
	/*
	while (tmpDataIdx < sm_headLen){
		tmpHeadBuf[tmpDataIdx] = sm_cKey[tmpDataIdx] ^ result[tmpDataIdx + sm_headLen];
		++tmpDataIdx;
	}
	*/
	if (memcmp(data, tmpHeadBuf, sm_headLen) == 0){
		if (tmpDataLen<10){
			tmpDataIdx = 0;
			while (tmpDataIdx < tmpDataLen){
				if (tempKeyLenIdx >= sm_keyLen) {
					tempKeyLenIdx = 0;
				}
				result[tmpDataIdx] = data[sm_headLen + tmpDataIdx] ^ sm_cKey[tempKeyLenIdx];
				++tempKeyLenIdx;
				++tmpDataIdx;
			}
		}
		else{
			unsigned int tmpAllLen = tmpDataLen;
			/************************************************************************/
			/* 奇
			/************************************************************************/
			if (tmpDataLen & 1){
				tmpAllLen -= 1;
			}
			//start encode
			tmpDataIdx = 0;
			for (tmpDataIdx = 0; tmpDataIdx < tmpAllLen; tmpDataIdx += 2){
				if (tempKeyLenIdx >= sm_keyLen) {
					tempKeyLenIdx = 0;
				}
				result[tmpDataIdx] = data[sm_headLen+tmpDataIdx] ^ sm_cKey[tempKeyLenIdx];
				result[tmpDataIdx + 1] = data[sm_headLen + tmpDataIdx + 1];
				++tempKeyLenIdx;
			}
			if (tmpDataLen & 1){
				if (tempKeyLenIdx >= sm_keyLen) {
					tempKeyLenIdx = 0;
				}
				result[tmpAllLen] = data[sm_headLen + tmpAllLen] ^ sm_cKey[tempKeyLenIdx];
			}
		}
		*ret_length = data_len - sm_headLen;
		result[*ret_length] = '\0';
		return result;
	}

	if (orgReturn == true){
		*ret_length = data_len;
		return data;
	}

	return nullptr;
}



void JOCrypto::decodeFile(const char *key, const char *filePath, const char *outPath)
{
    FILE *fp = fopen(filePath,"rb");
    if (fp)
    {
		JOCrypto::setKey(key);
		JOFileMgr::MkMulDir(JOPath::getFileBasePath(outPath));
		//std::string fileName = JOPath::getFileName(filePath);
		//JOFileMgr::MkMulDir(outPath);

        fseek(fp,0,SEEK_END);
        size_t size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        
        unsigned char * orgBuffer = (unsigned char*)malloc(size);
        size = fread(orgBuffer,sizeof(unsigned char), size,fp);
        
        unsigned int outSize;
        unsigned char *decodeBuff = JOCrypto::decode(orgBuffer, (unsigned int)size, &outSize, true);
        
		
		//std::string outPutFile = JOString::formatString("%s/%s", outPath, fileName.c_str());
		//FILE *fwp = fopen(outPutFile.c_str(),"wb");

		FILE *fwp = fopen(outPath, "wb");
        if (fwp) {
            size_t write = fwrite(decodeBuff, 1, (size_t)outSize, fwp);
            if (write <= 0) {
				printf("decode write to %s fail\n", outPath);
				JOFileMgr::removeFile(outPath);
            }
            fflush(fwp);
            fclose(fwp);
        }
		free(decodeBuff);
        fclose(fp);
    }
}
void JOCrypto::encodeFile(const char *key, const char *filePath, const char *outPath)
{
    FILE *fp = fopen(filePath,"rb");
    if (fp)
    {
		JOCrypto::setKey(key);
		JOFileMgr::MkMulDir(JOPath::getFileBasePath(outPath));
		//std::string fileName = JOPath::getFileName(filePath);
		//JOFileMgr::MkMulDir(outPath);
        fseek(fp,0,SEEK_END);
        size_t size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        
        unsigned char * orgBuffer = (unsigned char*)malloc(size);
        size = fread(orgBuffer,sizeof(unsigned char), size,fp);
        
        unsigned int outSize;
        unsigned char *encodeBuff = JOCrypto::encode(orgBuffer, (unsigned int)size, &outSize);
		
        //std::string outPutFile = JOString::formatString("%s/%s", outPath, fileName.c_str());
        //FILE *fwp = fopen(outPutFile.c_str(),"wb");

		FILE *fwp = fopen(outPath, "wb");
        if (fwp) {
            size_t write = fwrite(encodeBuff, 1, (size_t)outSize, fwp);
            if (write <= 0) {
				printf("encode write to %s fail\n", outPath);
				JOFileMgr::removeFile(outPath);
            }
            fflush(fwp);
            fclose(fwp);
        }
		free(encodeBuff);
        fclose(fp);
    }
}

NS_JOFW_END