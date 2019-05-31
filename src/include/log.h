#pragma once
#include <string>

inline std::string __s(const std::string &s) noexcept { return s; }

namespace Log
{
// 创建日志文件并且设置调试等级(默认无)
void InitLog(const int level);

// 根据调试等级向日志文件中写入日志
void WriteLog(const int level, const std::string &log);

// 关闭日志文件
void CloseLog();
} // namespace Log