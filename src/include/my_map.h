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

	bool insert(int id, const sockaddr &addr);
	sockaddr find(int id);
	bool erase(int id);

private:
	std::map<int, sockaddr> map_;
	std::shared_mutex mutex_;
};
