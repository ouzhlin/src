#include "module/dyer/JODyerParser.h"
#include "module/dyer/vo/JODyerVO.h"
#include "manager/JOTickMgr.h"
#include "manager/JOShaderMgr.h"
#include "manager/JOCachePoolMgr.h"
#include "core/datautils/JODataPool.h"
#include "core/datautils/JODataCoder.h"
#include "utils/JOPath.h"
#include "utils/JOTime.h"
#include "utils/JOLog.h"
#include "ui/JOSprite.h"

NS_JOFW_BEGIN


DyerData::DyerData(char lenght, float* h, float* s, float* l) :len(lenght), harr(h), sarr(s), larr(l)
{
}

DyerData::DyerData(char lenght) : len(lenght)
{
	harr = new float[len]();
	sarr = new float[len]();
	larr = new float[len]();
}

DyerData::~DyerData()
{
	if (harr)
	{
		delete harr;
		harr = nullptr;
	}
	if (sarr)
	{
		delete sarr;
		sarr = nullptr;
	}
	if (larr)
	{
		delete larr;
		larr = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////


JODyerParser::JODyerParser()
{

}

JODyerParser::~JODyerParser()
{
	clear();
}

void JODyerParser::saveToFile(const std::string& path, DyerCaseVec& vec)
{
	unsigned int size = vec.size();
	unsigned int len = 0;
	if (size > 0){
		DyerData* dd = vec.front();
		len = dd->len;
	}
	else{
		return;
	}
	unsigned int total = len *sizeof(float) * 3 * size + 2;
	unsigned char* buff = new unsigned char[total];
	unsigned int buffseek = 0;	
	//memcpy(buff + buffseek, (char*)size, 1);	
	buff[buffseek] = size;
	buffseek += 1;

	//memcpy(buff + buffseek, (char*)len, 1);
	buff[buffseek] = len;
	buffseek += 1;

	unsigned int offset = len*sizeof(float);
	for (unsigned int i = 0; i < size; i++)
	{
		DyerData* dd = vec[i];		
		memcpy(buff + buffseek, dd->harr, offset);
		buffseek += offset;

		memcpy(buff + buffseek, dd->sarr, offset);
		buffseek += offset;

		memcpy(buff + buffseek, dd->larr, offset);
		buffseek += offset;
	}

	FILE* fp = fopen(path.c_str(), "wb");
	if (fp)
	{
		fwrite(buff, buffseek, 1, fp);
		fclose(fp);		
	}
	delete [] buff;
}

bool JODyerParser::getDataFromFile(const std::string& path, DyerCaseVec& vec)
{
	if (!FileUtils::getInstance()->isFileExist(path)){
		return false;
	}
	Data d = FileUtils::getInstance()->getDataFromFile(path);
	if (d.isNull() || d.getSize()<3){
		return false;
	}
	unsigned char* buff = d.getBytes();
	/*
	
	unsigned char caseCount = _readUnsignedChar(buff);
	buff += 1;	
	unsigned char len = _readUnsignedChar(buff);
	*/
	JODataCoder* coder = JODataPool::Instance()->getDataCoder();
	coder->init(d.getBytes(), d.getSize());
	unsigned int caseCount = coder->readUByte();
	buff += 1;
	unsigned int len = coder->readUByte();
	buff += 1;
	vec.resize(caseCount);
	if (caseCount < 1 || len < 1)
	{
		JODataPool::Instance()->recover(coder);
		return false;
	}
	unsigned int offset = len*sizeof(float);
	for (unsigned int i = 0; i < caseCount; i++)
	{
		
		float *h = new float[len];
		float *s = new float[len];
		float *l = new float[len];

		memcpy(h, buff, offset);
		buff += offset;
		memcpy(s, buff, offset);
		buff += offset;
		memcpy(l, buff, offset);
		buff += offset;

		DyerData *dd = new DyerData(len, h, s, l);
		vec[i] = dd;
	}
	JODataPool::Instance()->recover(coder);
	return true;
}


bool JODyerParser::dyer(JOSprite* sprite, unsigned char solutionId)
{
	return dyer(sprite, sprite->getSourcePath(), solutionId);
}

bool JODyerParser::dyer(cocos2d::Sprite* sprite, const std::string& srcPath, unsigned char solutionId)
{	
	std::string fdir = JOPath::getFileBasePath(srcPath);
	std::string fname = JOPath::getFileName(srcPath, false);
	
	// find dyer data
	std::string solutionPath = fdir + fname;
	std::vector<DyerData*> vec;	
	DYER_SOLUTION_MAP::iterator itr = dyer_solution_map.find(solutionPath);
	if (itr == dyer_solution_map.end())	{		
		if (!getDataFromFile(solutionPath + ".sol", vec)){
			return false;
		}
		dyer_solution_map[solutionPath] = vec;
	}
	else{
		vec = itr->second;
	}

	if (solutionId < vec.size()){		
		JOSprite* spr = nullptr;
		// find the block
		BLOCK_MAP::iterator itr = m_blockMap.find(solutionPath);
		if (itr == m_blockMap.end()){
			spr = JOSprite::create(fname + ".block", true, [=](cocos2d::Node* sender, std::string sourcePath, std::string imgName){
				// wait for block
				m_blockDoneList.push_back(solutionPath);
			});
			// pre set block
			JODyerBlockVO* vo = POOL_GET(JODyerBlockVO, "JODyerParser");
			vo->init(spr, JOTime::getTimeofday(), solutionPath);
			m_blockMap[solutionPath] = vo;
		}
		else{
			// finded the block
			JODyerBlockVO* blockVo = m_blockMap[solutionPath];
			blockVo->m_time = JOTime::getTimeofday();

			if (!blockVo->m_block->isAsynLoading()){
				// set the dyer now
				_setDyer(sprite, blockVo->m_block->getTexture(), vec[solutionId]);
				return true;
			}
			// put the dyer target to list
			JODyerVO* vo = POOL_GET(JODyerVO, "JODyerParser");
			vo->init(sprite, vec[solutionId], solutionPath);
			DYERs_MAP::iterator itr = m_dyersMap.find(solutionPath);
			if (itr == m_dyersMap.end()){
				std::list<JODyerVO*> list;
				m_dyersMap[solutionPath] = list;
			}
			m_dyersMap[solutionPath].push_back(vo);
		}
		return true;
	}
	return false;
}

bool JODyerParser::_setDyer(cocos2d::Sprite* spr, cocos2d::Texture2D* tex, DyerData* dd)
{
	if (spr==nullptr){
		LOG_WARN("JODyerParser", "src Sprite is null!!!");
		return false;
	}
	if (tex == nullptr){
		LOG_WARN("JODyerParser", "block tex is null!!!");
		return false;
	}
	if (dd == nullptr){
		LOG_WARN("JODyerParser", "DyerData is null!!!");
		return false;
	}
	JOShaderMgr::Instance()->shader(spr, "dyer", [=](cocos2d::Node* target, cocos2d::GLProgramState* pState){

		pState->setUniformTexture("u_dTex", tex);

		GLProgram* program = pState->getGLProgram();
		GLint hs = program->getUniformLocationForName("u_dHs");
		GLint ss = program->getUniformLocationForName("u_dSs");
		GLint ls = program->getUniformLocationForName("u_dLs");

		program->setUniformLocationWith1fv(hs, dd->harr, dd->len);
		program->setUniformLocationWith1fv(ss, dd->sarr, dd->len);
		program->setUniformLocationWith1fv(ls, dd->larr, dd->len);
	});
	return true;		 
}

void JODyerParser::unDyer(cocos2d::Sprite* sprite, const std::string& srcPath)
{
	std::string path = JOPath::getFileWithoutSuffix(srcPath);

	DYERs_MAP::iterator itr = m_dyersMap.find(path);
	if (itr != m_dyersMap.end()){
		std::list<JODyerVO*>::iterator listItr = itr->second.begin();
		while (listItr != itr->second.end()){
			JODyerVO* vo = *listItr;
			if (vo->m_ori == sprite){
				vo->done();
				POOL_RECOVER(vo, JODyerVO, "JODyerParser");
				itr->second.erase(listItr);
				if (itr->second.empty()){
					m_dyersMap.erase(itr);
				}
				break;
			}
			++listItr;
		}
	}
	JOShaderMgr::Instance()->restore(sprite);
}

void JODyerParser::unDyer(JOSprite* sprite)
{
	unDyer(sprite, sprite->getSourcePath());
}

void JODyerParser::tick()
{
	if (!m_blockDoneList.empty()){
		std::list<std::string>::iterator itr = m_blockDoneList.begin();
		while (itr != m_blockDoneList.end()){
			JODyerBlockVO* blockVo = m_blockMap[*itr];
			if (blockVo){
				std::list<JODyerVO*>* list = &m_dyersMap[*itr];
				std::list<JODyerVO*>::iterator listItr = list->begin();
				while (listItr != list->end()){
					JODyerVO* vo = *listItr;
					_setDyer(vo->m_ori, blockVo->m_block->getTexture(), vo->m_dd);
					vo->done();
					POOL_RECOVER(vo, JODyerVO, "JODyerParser");
					++listItr;
				}
				m_dyersMap.erase(*itr);
			}
			++itr;
		}
		m_blockDoneList.clear();
	}

	static float totalInterval = 0;
	totalInterval += JOTickMgr::Instance()->deltaTime();
	if (totalInterval > 200){
		totalInterval = 0;
		_checkBlockCache();
	}
}

void JODyerParser::_checkBlockCache()
{
	double tmpCurTime = JOTime::getTimeofday();
	BLOCK_MAP::iterator itr = m_blockMap.begin();
	JODyerBlockVO* vo = nullptr;
	while (itr != m_blockMap.end()){
		vo = itr->second;
		if ((tmpCurTime-vo->m_time)>180){
			vo->done();
			POOL_RECOVER(vo, JODyerBlockVO, "JODyerParser");
			itr = m_blockMap.erase(itr);
		}
		else{
			++itr;
		}
	}
}

void JODyerParser::clear()
{
	BLOCK_MAP::iterator itr = m_blockMap.begin();
	JODyerBlockVO* blockVo = nullptr;
	while (itr != m_blockMap.end()){
		blockVo = itr->second;
		blockVo->done();
		POOL_RECOVER(blockVo, JODyerBlockVO, "JODyerParser");
		++itr;
	}
	m_blockMap.clear();

	JODyerVO* vo = nullptr;
	DYERs_MAP::iterator dyerItr = m_dyersMap.begin();
	while (dyerItr != m_dyersMap.end()){
		std::list<JODyerVO*>::iterator listItr = dyerItr->second.begin();
		while (listItr != dyerItr->second.end()){
			vo = *listItr;
			vo->done();
			POOL_RECOVER(vo, JODyerVO, "JODyerParser");			
			++listItr;
		}
		++dyerItr;
	}
	m_dyersMap.clear();
}


NS_JOFW_END