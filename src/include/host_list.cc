#include <string>
#include <fstream>
#include <iostream>

#include "host_list.h"
#include "log.h"

const std::string HostList::banned_host_ = "0.0.0.0";

HostList::HostList(const std::string &path)
{
	if (Load(path))
	{
		Log::WriteLog(1, __s("host list loading succeed"));
	}
	else
	{
		Log::WriteLog(1, __s("host list loading failed"));
	}
}

static bool is_ipv6(const std::string &str)
{
	int p = 0;
	for (auto c : str)
	{
		if (c == '.')
			p++;
		else if (c == ':')
			return true;
	}
	if (p == 3)
		return false;
	else return true;
}

HostState HostList::get_host_state(const std::string &host_name, int net_type) const
{
	auto iter = host_map_.find(host_name); //在提供的配置文件生成的表中查询是否有传入变量名（Q.NAME）
	for (auto i = host_map_.begin(); i != host_map_.end(); ++i)
	{
		if ((*i).first == host_name)
		{
			if (is_ipv6((*i).second) && net_type == 1)
			{
				return FIND;
			}
			else (!is_ipv6((*i).second) && net_type == 0)
			{
				if ((*i).second == banned_host_)
					return BANNED;
				else
				{
					return FIND;
				}
			}
		}
	}
	return NOT_FIND;
	/*/
	if (iter == host_map_.end()) //到末尾，并未查询到
		return NOT_FIND;
	else if (iter->second == banned_host_) //查询到但域名为0.0.0.0
		return BANNED;
	else
		return FIND;
		*/
}

std::string HostList::get_ip_str(const std::string &host_name) const
{
	return host_map_.at(host_name);
}

bool HostList::Load(const std::string &path)
{
	std::ifstream stream(path); //创立文件输入流
	if (stream)					//创建成功
	{
		host_path_ = path;
		std::string host_ip, host_name;
		while (stream)
		{
			//文件每行记录以 ip地址+网址名 的方式给出
			stream >> host_ip;
			stream >> host_name;
			//std::cout << host_ip << " " << host_name << std::endl;
			if (stream)
				host_map_.insert({host_name, host_ip}); //向创建的map中插入关联组<网址名,ip地址>
		}
		return true;
	}
	else
	{
		Log::WriteLog(1, __s(" host list cannot open host file: ") + path);
		return false;
	}
}