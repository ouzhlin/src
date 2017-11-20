#include "manager/JOSnMgr.h"

NS_JOFW_BEGIN

JOSnMgr::JOSnMgr() :snApplyNum(0), maxSnAmount(100)
{
	_createSnCap();
}

JOSnMgr::~JOSnMgr()
{
	snDepotList.clear();
	snDepotSet.clear();
}

unsigned int JOSnMgr::getSn()
{
	if (snDepotList.empty()){
		++snApplyNum;
		_createSnCap();
	}
	unsigned int sn = snDepotList.front();
	snDepotList.pop_front();
	snDepotSet.erase(sn);
	return sn;
}

void JOSnMgr::dispose(unsigned int sn)
{
	std::unordered_set<unsigned int>::iterator itr = snDepotSet.find(sn);
	if (itr != snDepotSet.end()){
		return;
	}
	snDepotList.push_back(sn);
	snDepotSet.insert(sn);
}

void JOSnMgr::_createSnCap()
{
	unsigned int snVal;
	unsigned int initial = maxSnAmount*snApplyNum;
	if (initial<1){
		initial = 0;
	}

	unsigned int index = 0;
	while (index < maxSnAmount){
		snVal = initial + index;
		snDepotList.push_back(snVal);
		snDepotSet.insert(snVal);
		++index;
	}
}

NS_JOFW_END