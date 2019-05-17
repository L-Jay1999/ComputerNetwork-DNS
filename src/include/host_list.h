#pragma once

#include <unordered_map>
#include <string>

enum HostState
{
    BANNED = 0,
    FIND,
    NOT_FIND
};

class HostList
{
    public:
        HostList(std::string path = "../../data/hosts.txt");

		HostState find(const std::string& host_name);
		std::string get(const std::string& host_name);
    private:
        std::string host_path_;
		static const std::string banned_host_;
        std::unordered_map<std::string, std::string> host_map_; // <host_name, ip_addr>
};