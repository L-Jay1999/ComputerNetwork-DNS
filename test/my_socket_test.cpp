#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <utility>
#include <vector>
#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include "../src/include/my_socket.h"
#include "../src/include/dns_packet.h"


void print(const std::string &str)
{
	static std::mutex mut;
	std::lock_guard<std::mutex> lg(mut);
	std::cout << str << std::endl;
}

void send_test()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		print("SEND: WSAStartup failed");
		return;
	}

	MySocket sock_send(SEND_SOCKET);
	QueueData data;
	addrinfo hint, *res = nullptr;
	ZeroMemory(&hint, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_protocol = IPPROTO_UDP;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo("localhost", "10003", &hint, &res))
	{
		print("SEND: getaddrinfo failed");
		WSACleanup();
	}

	sockaddr_in *send_addr;
	send_addr = reinterpret_cast<sockaddr_in *>(res->ai_addr);
	char buffer[20];
	print(inet_ntop(res->ai_family, &(send_addr->sin_addr), buffer, 20));

	data.addr = *send_addr;
	data.len = 10;
	data.data = new char[data.len];
	for (int i = 0; i < data.len; i++)
		data.data[i] = '1' + i + 1;
	data.data[data.len - 1] = '\0';

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (sock_send.SendTo(data))
		{
			// print("SEND: sent packet");
		}
		else
		{
			print("SEND: error");
			delete[] data.data;
			freeaddrinfo(res);
			WSACleanup();
			return;
		}
	}
	return;
}

void recv_test()
{
	MySocket recv_sock(RECV_SOCKET, "10003");
	QueueData data;

	while (1)
	{
		data = recv_sock.RecvFrom();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (data.data)
		{
			print(data.data);
			delete[] data.data;
		}
		if (data.len == 0)
		{
			print("recv_error");
			return;
		}
	}

}

int main()
{
	std::thread recv_thread(recv_test);
	std::vector<std::thread> send_vec;
	
	for (std::size_t i = 0; i < 25; i++)
	{ 
		send_vec.push_back(std::move(std::thread(send_test)));
	}
	for (std::size_t i = 0; i < 25; i++)
	{
		send_vec[i].join();
	}
	recv_thread.join();
	return 0;
}


