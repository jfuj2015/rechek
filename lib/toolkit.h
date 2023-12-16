#ifndef __TOOLKIT_H__
#define __TOOLKIT_H__
#include <string>

namespace recheck {

	//单键函数
	template <class T> T* GetInstance()
	{
		static T* p = new T();
		return p;
	}



} //recheck




#endif // !__TOOLKIT_H__

