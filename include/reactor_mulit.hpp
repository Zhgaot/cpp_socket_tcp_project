#pragma once

#include "basic_tcp.h"
#include "thread_pool.hpp"
#include "reactor_base.hpp"
#include <sys/epoll.h>
#include <utility>
#include <stdint.h>
#include <string>
#include <iostream>

/* =============== Sub Reactor =============== */
/* --------------- Declaration --------------- */
class ReactorMulitSub : public Reactor {
public:
    explicit ReactorMulitSub(const uint32_t& epoll_size);
    void event_loop(const int events_num, const int epoll_timeout,
                    const std::string event_mode,
                    void(*call_back)(const int));
};
/* --------------- Declaration --------------- */


/* --------------- Definition --------------- */
inline ReactorMulitSub::ReactorMulitSub(const uint32_t& epoll_size) : Reactor(epoll_size) {}

void ReactorMulitSub::event_loop(const int events_num, const int epoll_timeout,
                    const std::string event_mode,
                    void(*call_back)(const int)){
    struct epoll_event events_out[events_num];
    while (true) {
        int count = epoll_wait(this->epoll_fd_,
                                events_out, events_num, epoll_timeout);
        if (count <= 0)
            continue;
        for (int i = 0; i < count; i++) {
            call_back(events_out[i].data.fd);
        }
    }
}
/* --------------- Definition --------------- */


/* =============== Main Reactor =============== */
/* --------------- Declaration --------------- */
class ReactorMulitMain : public Reactor {
public:
    explicit ReactorMulitMain(const uint32_t& epoll_size);
    void event_loop(const int events_num, const int epoll_timeout,
                    BasicTcp server,
                    const std::string event_mode, std::vector<ReactorMulitSub*> reactor_list,
                    void(*call_back)(const int, const std::string, const std::string));
};
/* --------------- Declaration --------------- */


/* --------------- Definition --------------- */
inline ReactorMulitMain::ReactorMulitMain(const uint32_t& epoll_size) : Reactor(epoll_size) {}

// template <typename F, typename... Args> F &&call_back, Args &&...args
void ReactorMulitMain::event_loop(const int events_num, const int epoll_timeout, 
                               BasicTcp server,
                               const std::string event_mode, std::vector<ReactorMulitSub*> reactor_list,
                               void(*call_back)(const int, const std::string, const std::string)
                               ) {
    struct epoll_event events_out[events_num];
    this->add_event(server.sock, event_mode);
    while (true) {
        int count = epoll_wait(this->epoll_fd_,
                                events_out, events_num, epoll_timeout);
        if (count <= 0)
            continue;
        server.set_block(false);
        for (int i = 0; i < count; i++) {
            if (events_out[i].data.fd == server.sock) {
                while (true) {
                    BasicTcp client = server.accept_connection();
                    if (client.sock == -1)
                        break;
                    // this->add_event(client.sock, event_mode);
                    // reactor_list[this->conn_count++ % reactor_list.size()]->add_event(client.sock, event_mode);
                    // reactor_list[0]->add_event(client.sock, event_mode);
                    call_back(client.sock, client.ip, event_mode);
                }
            } else {
                std::cout << "The Main Reactor received an incorrect fd" << std::endl;
            }
        }
    }
}
/* --------------- Definition --------------- */
