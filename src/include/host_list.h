#pragma once

#include <unordered_map>
#include <string>

enum HostState
{
	BANNED = 0, //拦截
	FIND,		//找到
	NOT_FIND	//未找到
};

class HostList
{
public:
	//加载路径path
	HostList(const std::string &path = "../data/hosts.txt");

	//根据传入的host_name在配置文件中查找其是否存在并据此返回对应的HostState
	HostState get_host_state(const std::string &host_name, int net_type) const;

	//根据传入的host_name在map中寻找其所在的组并返回其ip地址
	std::string get_ip_str(const std::string &host_name) const;

private:
	bool Load(const std::string &path);
	std::string host_path_;									//文件所在路径
	static const std::string banned_host_;					//被拦截字符串
	std::unordered_map<std::string, std::string> host_map_; // <host_name, ip_addr>
};