#include "../src/include/my_map.h"

#include <mutex>
#include <iostream>
#include <string>

#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

MyMap test_map;

void print(const std::string &str)
{
	static std::mutex mutex_;
	std::lock_guard<std::mutex> lock(mutex_);
	std::cout << str << std::endl;
}

void inserter(sockaddr *s, int size)
{
	char buf[50];
	char buf2[50];
	while (1)
	{
		for (int i = 0; i < size; i++)
		{
			if (test_map.insert(i, s[i]))
			{
				getnameinfo(&s[i], sizeof(*s), buf, 50, buf2, 50, NI_NUMERICSERV | NI_NUMERICHOST);
				print(std::string("insert ") + std::to_string(i) + " " + buf);
			}
		}
		Sleep(3000);
	}
}

void finder()
{
	char buf[50];
	char buf2[50];
	while (1)
	{
		for (int i = 0; i < 3; i++)
		{
			sockaddr temp = sockaddr();
			temp = test_map.find(i);
			if (temp.sa_family)
			{
				getnameinfo(&temp, sizeof(temp), buf, 50, buf2, 50, NI_NUMERICSERV | NI_NUMERICHOST);

				print(std::string("find ") + buf);
				if (test_map.erase(i))
					print(std::string("erase ") + std::to_string(i) + " " + buf);
			}
			
		}
	}
}


int main()
{
	int e;
	WSADATA wsaData;
	e = WSAStartup(MAKEWORD(2, 2), &wsaData);

	addrinfo hint, *res[3];
	ZeroMemory(&hint, sizeof(hint));
	hint.ai_flags = AI_PASSIVE;
	hint.ai_protocol = IPPROTO_UDP;
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_DGRAM;

	e = getaddrinfo("localhost", NULL, &hint, &res[0]);
	e = getaddrinfo("192.168.0.1", NULL, &hint, &res[1]);
	e = getaddrinfo("123.213.132.111", NULL, &hint, &res[2]);
	sockaddr s[3] = {*res[0]->ai_addr,*res[1]->ai_addr,*res[2]->ai_addr};

	std::thread i(inserter, s, 3), f(finder);
	i.join();
	f.join();
	return 0;
}