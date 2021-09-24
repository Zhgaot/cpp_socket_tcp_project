#pragma once

#include "basic_tcp.h"
#include "thread_pool.hpp"
#include <sys/epoll.h>
#include <utility>
#include <stdint.h>
#include <string>
#include <iostream>

/* --------------- Declaration --------------- */
class Reactor {
public:
    Reactor(const uint32_t& epoll_size);
    
    virtual bool add_event(const int& fd, const std::string& mode);
    virtual bool mod_event(const int& fd, const std::string& mode);
    virtual bool del_event(const int& fd);

protected:
    int epoll_fd_;
};
/* --------------- Declaration --------------- */


/* --------------- Definition --------------- */
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
/* --------------- Definition --------------- */
