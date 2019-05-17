#pragma once
#include <WinSock2.h>
class MySocket
{
public:
	MySocket();
private:
    static constexpr int recvbuflen = 512;
    char recvbuf[recvbuflen];
    sockaddr from;
    int from_len = sizeof(sockaddr);
    // ......
};