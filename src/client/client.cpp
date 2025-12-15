#include "client.hpp"

void thread_recv(int id_socket) {
    char buf[64];
    while (true) {
        int count_bytes;
        #ifdef _WIN32
        count_bytes = recv(id_socket, buf, 64, 0);
        #endif
        #ifdef __linux__
        count_bytes = read(id_socket, buf, 64, 0);
        #endif
        for (int i = 0; i < count_bytes; i++) {
            std::cout << buf[i];
        }
        std::cout << std::endl;
        memset(buf, 0x00, 64);
    }
}
