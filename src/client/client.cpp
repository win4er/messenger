#include "client.hpp"

const int BUF_SIZE = 128;
char BUFFER[BUF_SIZE] = {0};

void thread_recv(int id_socket) {
	// infinitive loop for receiving data from server
	// later i will fix it, trust me
    int count_bytes;
    while (true) {
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
