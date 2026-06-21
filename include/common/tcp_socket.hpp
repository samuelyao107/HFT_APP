#pragma once

#include <functional>
#include "common/logger.hpp"

namespace common
{

  constexpr size_t TCPBufferSize = 64 * 1024 * 1024;

  struct TCPSocket
  {

  public:
    explicit TCPSocket(Logger &logger);

    int fd_ = -1;

    char *send_buffer_ = nullptr;
    char *recv_buffer_ = nullptr;
    size_t next_send_valid_index_ = 0;
    size_t next_recv_valid_index_ = 0;

    bool send_disconnected = false;
    bool recv_disconnected = false;

    struct sockaddr_in inInAddr{};

    std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_;

    std::string time_str;
    Logger &logger_;

    auto connect(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int;
    auto send(const char *data, size_t len) noexcept -> void;
    auto sendAndRecv() noexcept -> bool;

    auto defaultRecvCallback(TCPSocket *socket, Nanos rx_time) noexcept -> void;
    auto destroy() noexcept -> void;

    ~TCPSocket();

    TCPSocket() = delete;
    TCPSocket(const TCPSocket &) = delete;
    TCPSocket(TCPSocket &&) = delete;
    TCPSocket &operator=(const TCPSocket &) = delete;
    TCPSocket &operator=(TCPSocket &&) = delete;
  }
}
