#include "utils/JOJsonHelper.h"

#include "utils/JOLog.h"

#include "external/json/stringbuffer.h"
#include "external/json/writer.h"
//#include "utils/rapidjson/stringbuffer.h"
//#include "utils/rapidjson/writer.h"
#include <iostream>

NS_JOFW_BEGIN

bool JOJsonHelper::getFileDictionary(RAPIDJSON_NAMESPACE::Document &root, const char* jsonFilePath)
{
	FILE *fp = fopen(jsonFilePath, "rt");
	if (fp)
	{
		unsigned int size = 0;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (size < 1)
		{
			return false;
		}

		unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * (size + 1));
		buffer[size] = '\0';


		unsigned int readsize = fread(buffer, sizeof(unsigned char), size, fp);
		fclose(fp);

		if (readsize < 1)
		{
			return false;
		}
		if (readsize < size)
		{
			buffer[readsize] = '\0';
		}
		return getStringDictionary(root, (const char*)buffer);
	}
	return false;
}


bool JOJsonHelper::getStringDictionary(RAPIDJSON_NAMESPACE::Document &root, const char* jsonString)
{
	RAPIDJSON_NAMESPACE::Document jsonDict;
	root.Parse<0>(jsonString);
	if (root.HasParseError())
	{
		LOG_ERROR("JOJsonHelper", "GetParseError %s\n", root.GetParseError());
		return false;
	}
	return true;
	/*
	RAPIDJSON_NAMESPACE::StringBuffer buffer;
	RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buffer);
	jsonDict.Accept(writer);
	std::cout << buffer.GetString() << std::endl;
	*/
}


const RAPIDJSON_NAMESPACE::Value& JOJsonHelper::getSubDictionary(const RAPIDJSON_NAMESPACE::Value &root, const char* key)
{
	return root[key];
}

const RAPIDJSON_NAMESPACE::Value& JOJsonHelper::getSubDictionary(const RAPIDJSON_NAMESPACE::Value &root, const char* key, int idx)
{
    return root[key][idx];
}

const RAPIDJSON_NAMESPACE::Value& JOJsonHelper::getSubDictionary(const RAPIDJSON_NAMESPACE::Value &root, int idx)
{
    return root[idx];
}

int JOJsonHelper::getIntValue(const RAPIDJSON_NAMESPACE::Value& root, const char* key, int def)
{
    int nRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(key));
        JO_BREAK_IF(root[key].IsNull());
        nRet = root[key].GetInt();
    } while (0);
    
    return nRet;
}


float JOJsonHelper::getFloatValue(const RAPIDJSON_NAMESPACE::Value& root,const char* key, float def)
{
	float fRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(key));
        JO_BREAK_IF(root[key].IsNull());
        fRet = (float)root[key].GetDouble();
    } while (0);
    
    return fRet;
}

bool JOJsonHelper::getBooleanValue(const RAPIDJSON_NAMESPACE::Value& root,const char* key, bool def)
{
    bool bRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(key));
        JO_BREAK_IF(root[key].IsNull());
        bRet = root[key].GetBool();
    } while (0);
    
    return bRet;
}

const char* JOJsonHelper::getStringValue(const RAPIDJSON_NAMESPACE::Value& root,const char* key, const char *def)
{
    const char* sRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(key));
        JO_BREAK_IF(root[key].IsNull());
        sRet = root[key].GetString();
    } while (0);
    
    return sRet;
}



int JOJsonHelper::getArrayCount(const RAPIDJSON_NAMESPACE::Value& root, const char* key, int def)
{
    int nRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(key));
        JO_BREAK_IF(root[key].IsNull());
        nRet = (int)(root[key].Size());
    } while (0);
    
    return nRet;
}


int JOJsonHelper::getIntValueFromArray(const RAPIDJSON_NAMESPACE::Value& root,const char* arrayKey,int idx, int def)
{
    int nRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(arrayKey));
        JO_BREAK_IF(root[arrayKey].IsNull());
        JO_BREAK_IF(root[arrayKey][idx].IsNull());
        nRet = root[arrayKey][idx].GetInt();
    } while (0);
    
    return nRet;
}


float JOJsonHelper::getFloatValueFromArray(const RAPIDJSON_NAMESPACE::Value& root,const char* arrayKey,int idx, float def)
{
    float fRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(arrayKey));
        JO_BREAK_IF(root[arrayKey].IsNull());
        JO_BREAK_IF(root[arrayKey][idx].IsNull());
        fRet = (float)root[arrayKey][idx].GetDouble();
    } while (0);
    
    return fRet;
}

bool JOJsonHelper::getBoolValueFromArray(const RAPIDJSON_NAMESPACE::Value& root,const char* arrayKey,int idx, bool def)
{
	bool bRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(arrayKey));
        JO_BREAK_IF(root[arrayKey].IsNull());
        JO_BREAK_IF(root[arrayKey][idx].IsNull());
        bRet = root[arrayKey][idx].GetBool();
    } while (0);
    
    return bRet;
}

const char* JOJsonHelper::getStringValueFromArray(const RAPIDJSON_NAMESPACE::Value& root,const char* arrayKey,int idx, const char *def)
{
    const char *sRet = def;
    do {
        JO_BREAK_IF(root.IsNull());
		//JO_BREAK_IF(!root.HasMember(arrayKey));
        JO_BREAK_IF(root[arrayKey].IsNull());
        JO_BREAK_IF(root[arrayKey][idx].IsNull());
        sRet = root[arrayKey][idx].GetString();
    } while (0);
    
    return sRet;
}

const RAPIDJSON_NAMESPACE::Value &JOJsonHelper::getDictionaryFromArray(const RAPIDJSON_NAMESPACE::Value &root, const char* key,int idx)
{
	return root[key][idx];
}

bool JOJsonHelper::checkObjectExist(const RAPIDJSON_NAMESPACE::Value &root)
{
    bool bRet = false;
    do {
        JO_BREAK_IF(root.IsNull());
        bRet = true;
    } while (0);
    
    return bRet;
}

bool JOJsonHelper::checkObjectExist(const RAPIDJSON_NAMESPACE::Value &root,const char* key)
{
    bool bRet = false;
    do {
        JO_BREAK_IF(root.IsNull());
        bRet = root.HasMember(key);
    } while (0);
    
    return bRet;
}

bool JOJsonHelper::checkObjectExist(const RAPIDJSON_NAMESPACE::Value &root, int index)
{
    bool bRet = false;
    do {   
        JO_BREAK_IF(root.IsNull());
        JO_BREAK_IF(!root.IsArray());
        JO_BREAK_IF(index < 0 || root.Size() <= (unsigned int )index);
        bRet = true;
    } while (0);

    return bRet;
}

NS_JOFW_END
