#include "manager/JOShaderMgr.h"
#include "manager/JOFileMgr.h"
#include "utils/JOLog.h"

#include "cocos2d.h"
USING_NS_CC;

NS_JOFW_BEGIN

static const std::string s_vsh_suffix = ".vsh";
static const std::string s_fsh_suffix = ".fsh";

JOShaderMgr::JOShaderMgr()
: m_glStateOriginal(nullptr)
, m_defaultVsh("")
{

}

JOShaderMgr::~JOShaderMgr()
{
	clearShaderSearchPath();
	CC_SAFE_RELEASE_NULL(m_glStateOriginal);
}

GLProgramState* JOShaderMgr::shader(Node* node, const char* key, std::function<void(Node*, cocos2d::GLProgramState*)> setValCall/* = nullptr*/)
{
	//GLProgram* program = GLProgramCache::getInstance()->getGLProgram(key);
	//if (program==nullptr){

	std::string fshRet = _getShaderBuffer(key + s_fsh_suffix);
	if (fshRet == ""){
		LOG_ERROR("JOShaderMgr", "key[%s] no shader fsh data!!!", key);
		return nullptr;
	}
    
    std::string vshRet = _getShaderBuffer(key + s_vsh_suffix);
    if (vshRet == "") {
        vshRet = m_defaultVsh;
    }


	GLProgram* program = GLProgram::createWithByteArrays(vshRet.c_str(), fshRet.c_str());
	if (program){
		GLProgramState* glState = GLProgramState::create(program);
		node->setGLProgramState(glState);
		if (setValCall)	{
			setValCall(node, glState);
		}
		return glState;
	}

	return nullptr;
}

void JOShaderMgr::restore(Node* node)
{
	if (node && m_glStateOriginal){
		node->setGLProgramState(m_glStateOriginal);
	}
}

void JOShaderMgr::setDefaultVsh(const char* key)
{
    std::string ret = _getShaderBuffer(key + s_vsh_suffix);
	if (ret != ""){
        m_defaultVsh = ret;
	}
	else{
		LOG_ERROR("JOShaderMgr", "key[%s] no default vsh data!!!", key);
	}
}

void JOShaderMgr::setDefaultOriginal(const char* key)
{
	std::string ret = _getShaderBuffer(key + s_fsh_suffix);
	if (ret == ""){
		LOG_ERROR("JOShaderMgr", "key[%s] no default original fsh data!!!", key);
		return;
	}

	const char*vsh = m_defaultVsh.c_str();
	const char*fsh = ret.c_str();
	GLProgram* program = GLProgram::createWithByteArrays(vsh, fsh);
	if (program==nullptr){
		LOG_ERROR("JOShaderMgr", "vsh [%s]\nfsh [%s]\ncan't create program!!!", vsh, fsh);
	}
	//GLProgramCache::getInstance()->addGLProgram(program, key);
	//}
	GLProgramState* glState = GLProgramState::create(program);
	if (m_glStateOriginal){
		m_glStateOriginal->release();
		m_glStateOriginal = nullptr;
	}
	m_glStateOriginal = glState;
	if (m_glStateOriginal){
		m_glStateOriginal->retain();
	}
}

void JOShaderMgr::addShaderSearchPath(const char* path)
{
	m_shaderPaths.push_back(path);
}

void JOShaderMgr::clearShaderSearchPath()
{
	m_shaderPaths.clear();
}

std::string JOShaderMgr::_getShaderBuffer(std::string key)
{
    std::unordered_map<std::string, std::string>::iterator itr = m_shaderBufferMap.find(key);
    if (itr == m_shaderBufferMap.end()) {
        JOData* d = _getShaderData(key);
        if (d){
            std::string ret((const char*)d->bytes(), d->length());
            m_shaderBufferMap[key] = ret;
            return ret;
        }
        m_shaderBufferMap[key] = "";
    }
    else{
        return itr->second;
    }
    return "";
}

JOData* JOShaderMgr::_getShaderData(std::string& key)
{
	JOData* d = nullptr;
	std::string path;
	std::vector<std::string>::iterator itr = m_shaderPaths.begin();
	while (itr != m_shaderPaths.end()){
		path = (*itr + key);
		if (JOFileMgr::Instance()->isFileExist(path)){
			d = JOFileMgr::Instance()->getFileData(path);
			if (d){
				return d;
			}
		}
		++itr;
	}

	if (JOFileMgr::Instance()->isFileExist(key)){
		return JOFileMgr::Instance()->getFileData(key);
	}
	return nullptr;
}

NS_JOFW_END
