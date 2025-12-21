#include "client.hpp"

void thread_recv(int id_socket) {
	// infinitive loop for receiving data from server
	// later i will fix it, trust me
	const int BUF_SIZE = 32;
	extern char BUFFER[BUF_SIZE];
    while (true) {
        int count_bytes;
        #ifdef _WIN32
        count_bytes = recv(id_socket, BUFFER, BUF_SIZE, 0);
        #endif
        #ifdef __linux__
        count_bytes = read(id_socket, BUFFER, BUF_SIZE, 0);
        #endif
        for (int i = 0; i < count_bytes; i++) {
            std::cout << BUFFER[i];
        }
        std::cout << std::endl;
        memset(BUFFER, 0x00, BUF_SIZE);
    }
}
