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

	//向map_中插入新个体，其序号为id，所存储的地址为addr
	bool insert(unsigned short id, const sockaddr_in &addr);
	//根据序号id返回其存储的地址
	sockaddr_in get(unsigned short id);
	//根据id查找以这一id为序号的个体是否存在
	bool find(unsigned short id);
	//删除以id为序号的个体
	bool erase(unsigned short id);

private:
	std::map<unsigned short, sockaddr_in> map_;
	std::shared_mutex mutex_;
};
