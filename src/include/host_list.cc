#include <string>
#include <fstream>
#include <iostream>
#include "host_list.h"

const std::string HostList::banned_host_ = "0.0.0.0";

HostList::HostList(std::string path)
{
	host_path_ = position;
	std::ifstream stream(host_path_);
	if (stream.is_open())
	{
		//std::cout << "open!" << std::endl;
		std::string data[2];
		while (!stream.eof())
		{
			stream >> data[0];
			stream >> data[1];
			//std::cout << data[0] << " " << data[1] << std::endl;
			host_map_.insert(std::pair <std::string, std::string>(data[1], data[0]));
		}
	}
	else std::cout << "open error!" << std::endl;
}

HostState HostList::find(const std::string& host_name)
{
	auto iter = host_map_.find(host_name);
	if (iter == host_map_.end())return NOT_FIND;
	if (iter->second == banned_host_)return BANNED;
	else return FIND;
}

std::string HostList::get(const std::string& host_name)
{
	auto iter = host_map_.find(host_name);
	if (iter != host_map_.end())return iter->second;
}