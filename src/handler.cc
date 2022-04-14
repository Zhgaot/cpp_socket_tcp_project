#include "handler.h"
#include <string>
#include <string.h>
#include <iostream>
using namespace std;

RecvSendThread::RecvSendThread(BasicTcp client) : server_client_(client) {}

void RecvSendThread::recv_send(bool reply) {
    /* 循环读取客户端数据，并给予回复 */
    char recv_buf[1024] = {0};
    while (true) {
        // recvlen表示实际返回值
        int recvlen = server_client_.receive_msg(recv_buf, sizeof(recv_buf) - 1);
        // 若recv函数出错则直接跳出循环，关闭此链接
        if (recvlen <= 0) {
            cout << "An error occurred in the recv function!" << endl;
            break;
        }
        else {
            recv_buf[recvlen] = '\0';
            cout << "Client msg: " << recv_buf << endl;
        }
        if (strstr(recv_buf, "quit") != nullptr || strstr(recv_buf, "exit") != nullptr) { // 当客户端需要退出时
            char send_msg[] = "quit success!\n";
            server_client_.send_msg(send_msg, sizeof(send_msg) + 1);
            cout << "A client has exited~~~" << endl;
            break;
        }
        if (reply) {
            cout << "The information sent by the client is: " << recv_buf << endl;
            string send_msg_input = "";
            cout << "Please enter the reply information: ";
            cin >> send_msg_input;
            char send_msg[1024];
            strcpy(send_msg, send_msg_input.c_str());
            int send_size = strlen(send_msg);
            int send_len = server_client_.send_msg(send_msg, send_size);
        } else {
            char send_msg[1024] = "[ECHO]: ";
            memcpy(send_msg + 8, recv_buf, recvlen);
            int send_size = 8 + recvlen;
            int send_len = server_client_.send_msg(send_msg, send_size);
            // break;
            /*  由于recv_buf结尾为'\0'，这里使用string是有bug的！会出现乱码
            string recv_msg_str(recv_buf);
            string send_msg_str = "[ECHO]: " + recv_msg_str;
            const char* send_msg = send_msg_str.c_str();
            int send_size = strlen(send_msg);
            int send_len = server_client_.send_msg(send_msg, send_size);
            break;
            */
        }
    }
    server_client_.close_socket();
    delete this;
}
