#pragma once

#include "TcpSocket.h"

namespace iv {
	/// Reads single characters or lines from a socket.
	/// Also writes strings to a socket.
    class BufferedSocketReaderWriter {
    private:
        TcpSocket m_socket;
        char* m_buf;
        int m_buf_max_len;
        int m_buf_pos;
        int m_buf_len;

    public:
        BufferedSocketReaderWriter(TcpSocket socket, int buffer_length) {
			m_buf_max_len = buffer_length;
			m_buf = new char[m_buf_max_len];

			m_buf_len = 0;
			m_buf_pos = 0;

			m_socket = std::move(socket);
		}

        ~BufferedSocketReaderWriter() {
			delete[] m_buf;
		}

		/// Returns the next character from the socket's stream, or -1 if an error or the end of stream is encountered.
		/// If `peek` == true, the character is not removed from the stream.
        int read(bool peek = false) {
			if (m_buf_pos >= m_buf_len) {
				m_buf_pos = 0;
				m_buf_len = m_socket.recv(m_buf, m_buf_max_len);

				if (m_buf_len <= 0) { // Error or eof
					return -1;
				}
			}

			if (peek) {
				return m_buf[m_buf_pos];
			}
			
			return m_buf[m_buf_pos++];
		}
		
		/// Reads until CRLF/CR/LF, or EOF, whichever comes first. CRLF/CR/LF is consumed, but not returned.
		std::string read_line() {
			std::string out;
			
			int ch;
			while ((ch = read()) != -1) {
				if (ch == '\r') { // CR
					if (read(true) == '\n') {
						read(); // Consume LF following the CR
					}
					return out;
				} else if (ch == '\n') { // LF
					return out;
				}

				out += (char) ch;
			}

			return out;
		}
		
		/// Returns number of characters written.
		int write(std::string str) {
			return m_socket.send(str.c_str(), str.size());
		}
    };
}
