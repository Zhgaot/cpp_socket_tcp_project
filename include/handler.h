#pragma once

#include "basic_tcp.h"

class RecvSendThread
{
public:
    RecvSendThread(BasicTcp client);
    void recv_send(bool reply);

private:
    BasicTcp server_client_;
};
