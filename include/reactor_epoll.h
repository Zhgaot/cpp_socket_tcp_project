#pragma once

#include "basic_tcp.h"
#include "thread_pool.hpp"
#include "handler.h"
#include <sys/epoll.h>
#include <utility>
#include <stdint.h>
#include <string>
#include <iostream>

class Reactor {
public:
    explicit Reactor(const uint32_t& epoll_size);
    
    bool add_event(const int& fd, const std::string& mode);
    bool mod_event(const int& fd, const std::string& mode);
    bool del_event(const int& fd);

    template <typename F, typename... Args>
    void event_loop(const int& events_num, 
                    const int& epoll_timeout, 
                    const uint32_t& loop_timeout,
                    BasicTcp& server,
                    const std::string& event_mode,
                    F &&call_bcak, Args &&...args);

private:
    int epoll_fd_;
};

inline Reactor::Reactor(const uint32_t& epoll_size) {
    this->epoll_fd_ = epoll_create(epoll_size);
}

inline bool Reactor::add_event(const int& fd, const std::string& mode){
    epoll_event ev;
    ev.data.fd = fd;
    if (mode == "ET")
        ev.events = EPOLLIN | EPOLLET;
    else if (mode == "LT") {}
    else {
        std::cout << "The mode of epoll is not valid!" << std::endl;
        return false;
    }
    epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    return true;
}

inline bool Reactor::mod_event(const int& fd, const std::string& mode){
    epoll_event ev;
    ev.data.fd = fd;
    if (mode == "ET")
        ev.events = EPOLLIN | EPOLLET;
    else if (mode == "LT") {}
    else {
        std::cout << "The mode of epoll is not valid!" << std::endl;
        return false;
    }
    epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    return true;
}

inline bool Reactor::del_event(const int& fd){
    epoll_event ev;
    ev.data.fd = fd;
    epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, fd, &ev);
    return true;
}

template <typename F, typename... Args>
void Reactor::event_loop(const int& events_num, 
                const int& epoll_timeout, 
                const uint32_t& loop_timeout,
                BasicTcp& server,
                const std::string& event_mode,
                F &&call_bcak, Args &&...args) {
    struct epoll_event events_out[events_num];
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
                    this->add_event(client.sock, event_mode);
                }
            } else {
                BasicTcp client;
                client.sock = events_out[i].data.fd;
                RecvSendThread* temp_reply = new RecvSendThread(client);
                call_bcak(temp_reply);
            }
        }
    }
}
