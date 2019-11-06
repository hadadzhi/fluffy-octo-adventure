#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <exception>
#include <cstring>
#include <string>

#include "TcpSocket.h"

namespace iv {
    /// Wraps a server TCP socket -- to listen and accept client connections.
    /// Closes the socket when destroyed.
    class TcpServerSocket {
    private:
        static const int kBacklog = 128; // No idea what the value should be :(

        int m_fd;
        bool m_closed;

    public:
        /// Default unusable state
        TcpServerSocket() : m_fd{-1}, m_closed{true} {}

        /// Creates a socket and starts listening on the specified host and port
        TcpServerSocket(const std::string& host, int port) : m_closed{false} {
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

            if (ai_ret != 0) {
                throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(ai_ret));
            }

            if (::bind(m_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
                ::freeaddrinfo(ai);
                throw std::runtime_error(std::string("bind: ") + std::strerror(errno));
            }

            ::freeaddrinfo(ai);

            if (::listen(m_fd, kBacklog) < 0) {
                throw std::runtime_error(std::string("listen: ") + std::strerror(errno));
            }
        }

        TcpServerSocket& operator=(TcpServerSocket&& other) {
            if (this == &other) {
                return *this;
            }

            m_fd = other.m_fd;
            m_closed = other.m_closed;

            other.m_fd = -1;
            other.m_closed = true;
        }

        TcpServerSocket(TcpSocket&& other) {
            *this = std::move(other);
        }

        ~TcpServerSocket() {
            close();
        }

        TcpSocket accept() {
            int fd = ::accept(m_fd, nullptr, nullptr);
            return TcpSocket(fd);
        }

        void close() {
            if (!m_closed) {
                ::close(m_fd);
                m_closed = true;
            }
        }

        int shutdown() {
            return ::shutdown(m_fd, SHUT_RDWR);
        }

        int fd() {
            return m_fd;
        }
    };
}
