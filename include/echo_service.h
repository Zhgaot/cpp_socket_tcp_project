#pragma once

#include "basic_tcp.h"

class TcpRecvSendThread
{
public:
    TcpRecvSendThread(BasicTcp client);
    void recv_send(bool reply);

private:
    BasicTcp server_client_;
};
