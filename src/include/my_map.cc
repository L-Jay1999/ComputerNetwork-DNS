#include <map>
#include <shared_mutex>

#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "my_map.h"

bool MyMap::insert(unsigned short id, const sockaddr_in &addr)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);
	if (!map_.count(id))
	{
		map_[id] = addr;
		return true;
	}
	else
		return false;
}

bool MyMap::find(unsigned short id)
{
	std::shared_lock<std::shared_mutex> lock(mutex_);
	if (map_.count(id))
		return true;
	else
		return false;
}

bool MyMap::erase(unsigned short id)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);
	if (map_.count(id))
	{
		map_.erase(id);
		return true;
	}
	else
	{
		return false;
	}
}

sockaddr_in MyMap::get(unsigned short id)
{
	std::shared_lock<std::shared_mutex> lock(mutex_);
	if (map_.count(id))
		return map_.at(id);
	else
		return sockaddr_in();
}