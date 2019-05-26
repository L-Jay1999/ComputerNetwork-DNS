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

HostState HostList::get_host_state(const std::string &host_name) const
{
	auto iter = host_map_.find(host_name);

	if (iter == host_map_.end())
		return NOT_FIND;
	else if (iter->second == banned_host_)
		return BANNED;
	else
		return FIND;
}

std::string HostList::get_ip_str(const std::string &host_name) const
{
	return host_map_.at(host_name);
}

bool HostList::Load(const std::string &path)
{
	std::ifstream stream(path);
	if (stream)
	{
		host_path_ = path;
		std::string host_ip, host_name;
		while (stream)
		{
			stream >> host_ip;
			stream >> host_name;
			//std::cout << data[0] << " " << data[1] << std::endl;
			if (stream)
				host_map_.insert({host_name, host_ip});
		}
		return true;
	}
	else
	{
		Log::WriteLog(1, __s(" host list cannot open host file: ") + path);
		// log write std::cout << "open error!" << std::endl;
		return false;
	}
}