#pragma once

#include "BlockingQueue.h"
#include "BufferedSocketReaderWriter.h"
#include "TcpServerSocket.h"

#include <string>
#include <fstream>
#include <atomic>
#include <thread>
#include <iostream>

namespace iv {
    /// Accepts HTTP requests, always answers with 200, logs request headers in five different files using five workers.
    class HttpLogger {
    private:
        static const int kBufSize = 512; // recv buffer size
        static const int kQueueSize = 10;

        iv::BlockingQueue<std::string> m_queue{kQueueSize};

        std::vector<std::thread> m_workers;
        std::thread m_listener;

        std::atomic_bool m_shutting_down;

    public:
        /// Initializes the HttpLogger and starts listening on the specified host and port.
        HttpLogger(const std::string& host, int port) : m_shutting_down{false} {
            m_listener = std::thread([&]() {
                iv::TcpServerSocket server_socket(host, port);

                while (!m_shutting_down) {
                    // Have to use select() to be able to timeout :(
                    fd_set read_fd_set;
                    FD_ZERO(&read_fd_set);
                    FD_SET(server_socket.fd(), &read_fd_set);

                    struct timeval timeout{};
                    timeout.tv_sec = 1;
                    timeout.tv_usec = 0;

                    int select_ret = ::select(FD_SETSIZE, &read_fd_set, nullptr, nullptr, &timeout);

                    if (select_ret == 0) { // Timeout
                        continue;
                    }

                    if (select_ret < 0) { // Error
                        std::cerr << "select: " << std::strerror(errno) << '\n';
                        if (errno == EINTR) { // Interrupted by a signal, give up
                            return;
                        }
                        continue;
                    }

                    TcpSocket client_socket = server_socket.accept();
                    if (!client_socket.valid()) {
                        std::cerr << "accept: " << std::strerror(errno) << '\n';
                        continue;
                    }

                    iv::BufferedSocketReaderWriter rw(std::move(client_socket), kBufSize);

                    std::string start_line = rw.read_line();
                    if (start_line.empty()) {
                        start_line = rw.read_line(); // Allow one empty line prior to start-line (RFC 7230, 3.5)
                    }
                    if (start_line.empty()) {
                        std::cerr << "ignoring invalid http message\n";
                        continue;
                    }

                    std::string headers;
                    std::string header;
                    while (!(header = rw.read_line()).empty()) { // Empty line separates the headers from the body
                        headers += header;
                        headers += '\n';
                    }

                    rw.write("HTTP/1.1 200 OK\r\n\r\n");

                    m_queue.put(headers);
                }
            });
            std::cerr << "listening on " << host << ':' << port << '\n';

            // Workers
            for (int i = 0; i < 5; ++i) {
                m_workers.emplace_back([&, i]() {
                    std::ofstream fs(std::string("worker-") + std::to_string(i) + ".log");

                    while (!m_shutting_down) {
                        std::string hs;

                        if (m_queue.take(hs)) {
                            fs << "===\n" << hs << std::endl; // flush the stream each time a set of headers is written
                        }
                    }
                });
                std::cerr << "worker " << i << " up\n";
            }
        }

        /// Requests this HttpLogger to shutdown
        void shutdown() {
            m_shutting_down = true;
            m_queue.begin_flush(); // Release workers blocked in take()
        }

        /// Blocks until this HttpLogger shuts down after a shutdown() request
        void join() {
            m_listener.join();
            for (auto& w : m_workers) {
                w.join();
            }
        }
    };
}
