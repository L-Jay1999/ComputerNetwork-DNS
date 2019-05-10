#pragma once

class MySocket
{
public:

private:
    static constexpr int recvbuflen = 512;
    char recvbuf[recvbuflen];
    sockaddr from;
    int from_len;
    // ......
};