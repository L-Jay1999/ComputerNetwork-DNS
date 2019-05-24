#pragma once
#include <string>

namespace Log
{
	void InitLog(const int level);
	void WriteLog(const int level,const std::string &log);
	void CloseLog();
}