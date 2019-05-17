#include <string>
#include <fstream>
#include <iostream>

#include "host_list.h"

const std::string HostList::banned_host_ = "0.0.0.0";

HostList::HostList(const std::string &path)
{
	if (Load(path))
	{
		// log write
	}
	else
	{
		// log write
	}
}

HostState HostList::get_host_state(const std::string& host_name) const
{
	auto iter = host_map_.find(host_name);

	if (iter == host_map_.end())
		return NOT_FIND;
	else if (iter->second == banned_host_)
		return BANNED;
	else 
		return FIND;
}

std::string HostList::get_ip_str(const std::string& host_name) const
{
	return host_map_.at(host_name);
}

bool HostList::Load(const std::string &path)
{
	std::ifstream stream(host_path_);
	if (stream)
	{
		host_path_ = path;
		// log wirte std::cout << "open!" << std::endl;
		std::string data[2];
		while (stream)
		{
			stream >> data[0];
			stream >> data[1];
			//std::cout << data[0] << " " << data[1] << std::endl;
			if (stream)
				host_map_.insert(std::pair <std::string, std::string>(data[1], data[0]));
		}
		return true;
	}
	else
	{
		// log write std::cout << "open error!" << std::endl;
		return false;
	}
}
