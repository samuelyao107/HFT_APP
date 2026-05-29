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

namespace Common
{
    constexpr int MaxTCPServerBacklog = 1024;

    inline auto join(int fd, const std::string &ip, const std::string &iface, int port) -> bool;

    // bind to a specific interface and get the IP address of that interface (do not want the OS to root for us)
    inline auto getIfaceIP(const std::string &iface) -> std::string
    {
        char buf[NI_MAXHOST] = {'\0'}; // NI_MAXHOST is a constant defined in <netdb.h> that specifies the maximum length of a hostname or IP address string. It is used to ensure that the buffer is large enough to hold the resulting string representation of the IP address.
        ifaddrs *ifaddr = nullptr;
        // the purpose of getifaddrs() is to retrieve the network interface addresses of the local machine. It populates a linked list of ifaddrs structures, each representing a network interface and its associated address information. The function returns 0 on success and -1 on error, and the result is stored in ifaddr, which must be freed with freeifaddrs() after use.
        if (getifaddrs(&ifaddr) != -1)
        { // getifaddrs() returns 0 on success, -1 on error,and the result is stored in ifaddr, which must be freed with freeifaddrs()
            for (ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next)
            {
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name)
                {
                    getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
                    break;
                }
            }
            freeifaddrs(ifaddr);
        }
        return std::string(buf);
    }

    inline auto setNonBlocking(int fd) -> bool
    {
        const auto flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            return false;
        }
        if (flags & O_NONBLOCK)
        {
            return true;
        }

        return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
    }
    // Disable Nagle Algorithm
    inline auto setNoDelay(int fd) -> bool
    {
        int one = 1; // Nagle's algorithm is a technique used in TCP/IP networking to improve network efficiency by reducing the number of small packets sent over the network. It works by combining multiple small outgoing messages into a single larger packet, which can help reduce overhead and improve overall throughput. However, in some cases, such as real-time applications or low-latency scenarios, it may be desirable to disable Nagle's algorithm to ensure that small packets are sent immediately without waiting for additional data to accumulate. The setsockopt() function is used to configure socket options, and in this case, it is being used to set the TCP_NODELAY option to disable Nagle's algorithm for the specified socket file descriptor (fd). The function returns true if the operation is successful (setsockopt() returns 0), and false otherwise.
        return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void *>(&one), sizeof(one)) != -1;
    }

    inline auto wouldBlock() -> bool
    {
        return errno == EWOULDBLOCK || errno == EINPROGRESS;
    }

    inline auto setTTL(int fd, int ttl) noexcept -> bool
    {
        return setsockopt(fd, IPPROTO_IP, IP_TTL, reinterpret_cast<void *>(&ttl), sizeof(ttl)) != -1;
    }

    inline auto setMcastTTL(int fd, int mcast_ttl) -> bool
    {
        return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void *>(&mcast_ttl), sizeof(mcast_ttl)) != -1;
    }

    inline auto setSOTimestamp(int fd) -> bool
    {
        int one = 1;
        return setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void *>(&one), sizeof(one)) != -1;
    }

    inline auto createSocket(Logger &logger, const std::string &t_ip, const std::string &iface, int port, bool is_udp,
                             bool is_blocking, bool is_listening, int ttl, bool needs_so_timestamp) -> int
    {

        std::string time_str;

        const auto ip = t_ip.empty() ? getIfaceIP(iface) : t_ip;
        logger.log("%:% %() % ip:% iface:% port:% is_udp:% is_blocking:% is_listening:% ttl:% SO_time:%\n",
                   __FILE__, __LINE__, __FUNCTION__,
                   Common::getCurrentTimeStr(&time_str), ip, iface, port,
                   is_udp, is_blocking, is_listening, ttl, needs_so_timestamp);
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = is_udp ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_protocol = is_udp ? IPPROTO_UDP : IPPROTO_TCP;
        hints.ai_flags = is_listening ? AI_PASSIVE : 0;  //AI_PASSIVE means my local host ip address
        if (std::isdigit(ip.c_str()[0]))
        {
            hints.ai_flags |= AI_NUMERICHOST;
        };
        hints.ai_flags |= AI_NUMERICSERV;

        addrinfo *result = nullptr;
        const auto rc = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);

        if (rc)
        {
            logger.log("getaddrinfo() failed. error:%\n", gai_strerror(rc), strerror(errno));
            return -1;
        }

        int fd = -1;
        int one = 1;
        for (addrinfo *rp = result; rp; rp = rp->ai_next)
        {
            fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (fd == -1)
            {
                logger.log("socket() failed. errno:%\n", strerror(errno));
                continue;
            }
            if (!is_blocking)
            {
                if (!setNonBlocking(fd))
                {
                    logger.log("setNonBlocking() failed. errno:%\n", strerror(errno));
                    close(fd);
                    fd = -1;
                    continue;
                };
                if (!is_udp && !setNoDelay(fd))
                {
                    logger.log("setNoDelay() failed. errno:%\n", strerror(errno));
                    close(fd);
                    fd = -1;
                    continue;
                }
            }
    
            if (!is_listening && connect(fd, rp->ai_addr, rp->ai_addrlen) == -1 && !wouldBlock())
            {
                logger.log("connect() failed. errno:%\n", strerror(errno));
                close(fd);
                fd = -1;
                continue;
            }

            if (is_listening && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&one),
                                      sizeof(one)); == -1)
            {
                logger.log("setsockopt() SO_REUSEADDR failed. errno:%\n", strerror(errno));
                close(fd);
                fd = -1;
                continue;
            }

            if (is_listening && bind(fd, rp->ai_addr, rp->ai_addrlen) == -1)
            {
                logger.log("bind() failed. errno:%\n", strerror(errno));
                close(fd);
                fd = -1;
                continue;
            }

            if (!is_udp && is_listening && listen(fd, MaxTCPServerBacklog) == -1)
            {
                logger.log("listen() failed. errno:%\n", strerror(errno));
                close(fd);
                fd = -1;
                continue;
            }

            if (is_udp && ttl)
            {
                const bool is_multicast = (atoi(ip.c_str()) & 0xe0) == 0xe0;
                if (is_multicast && !setMcastTTL(fd, ttl))
                {
                    logger.log("setMcastTTL() failed. errno:%\n", strerror(errno));
                    close(fd);
                    fd = -1;
                    continue;
                }
                if (is_multicast && !setTTL(fd, ttl))
                {
                    logger.log("setTTL() failed. errno:%\n", strerror(errno));
                    close(fd);
                    fd = -1;
                    continue;
                }
            }

            if (needs_so_timestamp && !setSOTimestamp(fd))
            {
                logger.log("setSOTimestamp() failed. errno:%\n", strerror(errno));
                close(fd);
                fd = -1;
                continue;
            }
            break;
        }

        if (result)
        {
            freeaddrinfo(result);
        }

        if (fd == -1)
        {
            logger.log("Failed to create and bind/connect socket across all getaddrinfo options.\n");
        }

        return fd;
    }

}
