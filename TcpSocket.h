#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <utility>
#include <string>
#include <cstring>

namespace iv {
    /// Wraps a client TCP socket -- to send/receive data.
    /// Closes the socket when destroyed.
    class TcpSocket {
    private:
        int m_fd;
        bool m_closed;

    public:
        /// Default unusable state
        TcpSocket() : m_fd{-1}, m_closed{true} {}

        /// Wraps a posix socket file descriptor
        explicit TcpSocket(int socked_fd) : m_fd{socked_fd}, m_closed{false} {}

        /// Creates and connects a socket
        TcpSocket(const std::string& host, int port) : m_closed{false} {
            m_fd = ::socket(PF_INET, SOCK_STREAM, 0);

            if (m_fd < 0) {
                throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
            }

            struct addrinfo hints{};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = 0;
            hints.ai_flags = 0;

            struct addrinfo* ai;
            int ai_ret = ::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &ai);

            if (ai_ret < 0) {
                throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(ai_ret));
            }

            if (::connect(m_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
                ::freeaddrinfo(ai);
                throw std::runtime_error(std::string("connect: ") + std::strerror(errno));
            }

            ::freeaddrinfo(ai);
        }

        TcpSocket& operator=(TcpSocket&& other) {
            if (this == &other) {
                return *this;
            }

            m_fd = other.m_fd;
            m_closed = other.m_closed;

            other.m_fd = -1;
            other.m_closed = true;
        }

        TcpSocket(TcpSocket&& other) {
            *this = std::move(other);
        }

        ~TcpSocket() {
            close();
        }

        int recv(void* buf, int len) {
            return ::recv(m_fd, buf, len, 0);
        }

        int send(const void* buf, int len) {
            return ::send(m_fd, buf, len, 0);
        }

        bool valid() {
            return m_fd >= 0;
        }

        void close() {
            if (!m_closed) {
                ::close(m_fd);
                m_closed = true;
            }
        }

        int fd() {
            return m_fd;
        }
    };
}
