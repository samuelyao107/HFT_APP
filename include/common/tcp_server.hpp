#pragma once

#include "tcp_socket.hpp"
#include "logging.hpp"

namespace Common
{

    struct TCPServer
    {
    public:
        int efd_ = -1;
        TCPSocket listener_socket_;

        epoll_event events_[1024];
        std::vector<TCPSocket *> sockets_, receive_sockets_, send_sockets_, disconnected_sockets_;

        std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_;
        std::function<void()> recv_finished_callback_;

        
        std::string time_str_;
        Logger &logger_;

        auto listen(const std::string &iface, int port) -> void;
        auto epoll_add(TCPSocket *socket); 
        auto epoll_del(TCPSocket *socket);
        auto del(TCPSocket *socket);
        auto poll()noexcept -> void;

        auto destroy();
        TCPServer(Logger &logger);

        TCPServer() = delete;
        TCPServer(const TCPServer &) = delete;
        TCPServer(const TCPServer &&) = delete;
        TCPServer &operator=(const TCPServer &) = delete;
        TCPServer &operator=(const TCPServer &&) = delete;
    }
}