#pragma once

#include <unordered_map>
#include <string>
#include <winsock2.h>

enum HostState
{
    BANNED = 0,
    FIND,
    NOT_FIND
};

class HostList
{
    public:
        HostList();

        HostState find(const std::string &host_name)
        {

        }

    private:
        static const std::string host_path_;
        std::unordered_map<std::string, unsigned> host_map_; // <host_name, ip_addr>
};