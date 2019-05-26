#pragma once
#include <map>
#include <shared_mutex>

#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

class MyMap
{
public:
	MyMap() = default;

	bool insert(unsigned short id, const sockaddr_in &addr);
	sockaddr_in get(unsigned short id);
	bool find(unsigned short id);
	bool erase(unsigned short id);

private:
	std::map<unsigned short, sockaddr_in> map_;
	std::shared_mutex mutex_;
};
