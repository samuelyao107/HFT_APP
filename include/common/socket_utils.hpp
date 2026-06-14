#pragma once

#include <iostream>
#include <string>
#include <unordered_set>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>

#include "macros.hpp"
#include "logging.hpp"

namespace Common{
    constexpr int MaxTCPServerBacklog =1024;

    auto join(int fd, const std::string &ip, const std::string &iface, int port) ->bool;
    auto createSocket(Logger &logger, const std::string &t_ip, const std::string &iface, int port, bool is_udp, 
                bool is_blocking, bool is_listening, int ttl, bool needs_so_timestamp) ->int;

    auto getIfaceIP(const std::string &iface) -> std::string {
        char buf[NI_MAXHOST] = {'\0'};
        ifaddrs *ifaddr = nullptr;

        if(getifaddrs(&ifaddr) != -1 ){
            for(ifaddrs *ifa = ifaddr; ifa ; ifa = ifa ->ifa_next){
                if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name){
                        getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf,sizeof(buf), NULL, 0, NI_NUMERICHOST);
                        break;
                }
            }
            freeifaddrs(ifaddr);
    };            
    return std::string(buf);
 }

 auto setNonBlocking(int fd) -> bool{
    const auto flags = fcntl(fd, F_GETFL,0);
    if(flags == -1){
        return false;
    }
    if(flags & O_NONBLOCK){
        return true;
    }

    return fcntl(fd,F_SETFL,flags | O_NONBLOCK) != -1 ;
 }
 //Disable Nagle Algorithm
 auto setNoDelay(int fd) -> bool{
    int one = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void*>(&one), sizeof(one)) !=-1;
 }
 
 auto wouldBlock() -> bool{
    return errno == EWOULDBLOCK || errno == EINPROGRESS ; 
 }

 auto setTTL(int fd, int ttl) noexcept -> bool{
    return setsockopt(fd, IPPROTO_TCP, IP_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) !=-1 ;
 }

 auto setMcastTTL(int fd, int mcast_ttl) ->bool{
    return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void*>(&mcast_ttl), sizeof(mcast_ttl)) !=-1 ;
 }

 auto setSOTimestamp(int fd) -> bool{
    int one =1;
    return setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void*>(&one), sizeof(one)) !=-1;
 }

}