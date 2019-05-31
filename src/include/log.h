#pragma once
#include <string>

inline std::string __s(const std::string &s) noexcept { return s; }

namespace Log
{
//根据level对日志进行初始化操作
void InitLog(const int level);
//根据level在日志中添加时间、进程号等信息
void WriteLog(const int level, const std::string &log);
//关闭日志
void CloseLog();
} // namespace Log