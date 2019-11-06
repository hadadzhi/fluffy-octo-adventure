#include <iostream>

// #include "BufferedSocketReaderWriter.h"
// #include "TcpSocket.h"
// #include "TcpServerSocket.h"
#include "HttpLogger.h"
#include <iostream>

int main() {
	
    // iv::TcpSocket socket("google.com", 80);
    // iv::BufferedSocketReaderWriter rw(std::move(socket), 512);

	// rw.write("GET / HTTP/1.1\r\n\r\n");

    // std::string start_line = rw.read_line();
    // if (start_line.empty()) {
        // start_line = rw.read_line(); // Allow one empty line prior to start-line (RFC 7230, 3.5)
    // }
    // if (start_line.empty()) {
        // std::cout << "Invalid HTTP message\n";
        // return -1;
    // }

    // std::string headers;
	// std::string header;
    // while (!(header = rw.read_line()).empty()) {
        // headers += header;
        // headers += '\n';
    // }
	
    // std::cout << "Start line: " << start_line << "\n\n===Headers===\n" << headers << '\n';

	iv::HttpLogger logger("*", 9090);
	
	std::cout << "Press `enter` to exit...\n";
	std::cin.get();
	
	logger.shutdown();
	logger.join();

    return 0;
}
