#pragma once
#include <string>

inline std::string __s(const std::string &s) { return s; }

namespace Log
{
	void InitLog(const int level);
	void WriteLog(const int level, const std::string &log);
	void CloseLog();
}