#include "common/tcp_socket.hpp"


namespace common {

    TCPSocket::TCPSocket(Logger &logger) : logger_(logger) {
        send_buffer_ = new char[TCPBufferSize];
        recv_buffer_ = new char[TCPBufferSize];
        recv_callback_ = [this](TCPSocket *socket, Nanos rx_time) {
            defaultRecvCallback(socket, rx_time);
        };
    }


    auto TCPSocket::connect(const std::string &ip, const std::string &iface, int port, bool is_listening) -> int {
        destroy();
        fd_ = createSocket(logger_,ip, iface, port, false,false, is_listening,0,true);

        inInAddr.sin_family = AF_INET;
        inInAddr.sin_port = htons(port);
        inInAddr.sin_addr.s_addr = INADDR_ANY;

        return fd_;
    }

    auto TCPSocket::send(const char *data, size_t len) noexcept -> void {
       if(len > 0){
            if (next_send_valid_index_ + len <= TCPBufferSize) {
                memcpy(send_buffer_ + next_send_valid_index_, data, len);
                next_send_valid_index_ += len;
            } else {
                // Log l'erreur : Buffer plein
                logger_.log("Buffer overflow prevented in TCPSocket::send\n");
            }
       }
    }

    auto TCPSocket::defaultRecvCallback(TCPSocket *socket, Nanos rx_time) noexcept -> void {
        logger_.log("%:%  %() % TCPSocket::defaultRecvCallback() socket:% len:% rx:%\n", 
                    __FILE__, __LINE__, __FUNCTION__, 
                    Common::getCurrentTimeStr(), socket->fd_, socket->next_recv_valid_index_, rx_time);
    }

    auto TCPSocket::destroy() noexcept -> void {
        close(fd_);
        fd_ = -1;
    }

    //Batching send and recv to reduce the number of system calls, which can improve performance in high-throughput scenarios.

    auto TCPSocket::sendAndRecv() noexcept -> bool {
       
        //Ancillary data buffer for receiving the timestamp from the kernel.
        
        char ctrl[CMSG_SPACE(sizeof(struct timeval))];
        struct cmsghdr *cmsg = (struct cmsghdr *)&ctrl;

        //scatter/gather I/O vector
        struct iovec iov; // iov = input/output vector
        iov.iov_base = recv_buffer_ + next_recv_valid_index_;//the base address of the buffer where the received data will be stored
        iov.iov_len = TCPBufferSize - next_recv_valid_index_;

        msghdr msg{};    //msg = messages header
        msg.msg_control = ctrl;
        msg.msg_controllen = sizeof(ctrl);
        msg.msg_name =  &inInAddr;
        msg.msg_namelen = sizeof(inInAddr);
        msg.msg_iov = &iov;// here we set the iovec structure to the msg structure, which will be used in the recvmsg() function to receive data from the socket.
        msg.msg_iovlen = 1;

        const auto n_recv = recvmsg(fd_, &msg, MSG_DONTWAIT); //return the number of bytes received, or -1 if an error occurred
        if(n_recv > 0){
            next_recv_valid_index_ += n_recv;

            Nanos kernel_time = 0;
            struct timeval time_kernel; //exact time of the packet arrival in the network card in seconds and microseconds
            if(cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMP  //SCM = Socket Control Message, SOL = Socket Option Level
            && cmsg->cmsg_len == CMSG_LEN(sizeof(time_kernel))){
                memcpy(&time_kernel, CMSG_DATA(cmsg), sizeof(time_kernel));
                kernel_time = time_kernel.tv_sec * NANOS_TO_SEC + time_kernel.tv_usec * NANOS_TO_MICROS;//timekernel separate seconds and microseconds, convert to nanoseconds
            }

            const auto user_time = getCurrentTimeNanos();
            logger_.log("%:% %() % read socket:% len:% ultime:% ktime:% diff:%\n",
                 __FILE__, __LINE__, __FUNCTION__, fd_,next_recv_valid_index_, user_time, kernel_time, user_time - kernel_time);
                 recv_callback_(this, kernel_time);
        }

        ssize_t n_send = std::min(TCPBufferSize, next_send_valid_index_);

        while(n_send>0){
            auto n_send_this_msg =std::min(static_cast<ssize_t> (next_send_valid_index_), n_send);
            const int flags = MSG_DONTWAIT | MSG_NOSIGNAL | (n_send_this_msg < n_send ? MSG_MORE : 0);
            auto n_ = ::send(fd_, send_buffer_, n_send_this_msg, flags);//returns the number of bytes sent, or -1 if an error occurred
            if(n_ <0)[[unlikely]]{
               if(!wouldBlock()){
                send_disconnected = true;
               }
               break;
            }

            logger_.log("%:% %() % send socket:% len:%\n", __FILE__, __LINE__, __FUNCTION__,Common::getCurrentTimeStr(&time_str), fd_, n_);
            n_send -= n_;
            ASSERT(n_ == n_send_this_msg, "Do not support partial send length yet.");

        }

        if (n_send == 0) {
            next_send_valid_index_ = 0;
        }

        return n_recv > 0;
    }

    TCPSocket::~TCPSocket() {
        destroy();
        delete[] send_buffer_; send_buffer_ = nullptr;
        delete[] recv_buffer_; recv_buffer_ = nullptr;
    }
}