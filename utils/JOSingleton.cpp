#include "utils/JOSingleton.h"

USING_NS_JOFW;

//template<typename T> JO_DLL T* JOSingleton<T>::s_singleton_ = nullptr;

//NS_JOFW_BEGIN


/*
JOSingletonBase::InstanceTable JOSingletonBase::s_instance_table_;

JOSingletonBase::InstanceTable::InstanceTable()
	:is_cleared_(false)
{
}

JOSingletonBase::InstanceTable::~InstanceTable()
{
	is_cleared_ = true;
	std::list < JOSingletonBase * >::reverse_iterator itr = this->rbegin();
	while (itr != this->rend()){
		JO_SAFE_DELETE(*itr);
		++itr;
	}
	this->clear();
}

JOSingletonBase::JOSingletonBase()
{
	//s_instance_table_.push_back(this);
}

JOSingletonBase::~JOSingletonBase()
{

	//if (!s_instance_table_.is_cleared_){

	//	InstanceTable::iterator itr = std::find(s_instance_table_.begin(), s_instance_table_.end(), this);

	//	if (itr != s_instance_table_.end())	{
	//		s_instance_table_.erase(itr);
	//	}
	//}
}
*/
//NS_JOFW_END