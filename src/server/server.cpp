#include "server.hpp"

// TODO:
// 1. make good argparser for this project
// 2. rebuild this shit, i dont like singletone(not essential)
// 3. make communication between clients (p2p)
// 4. make cipher-protocol
// 5. make first tui->gui

const int BUF_SIZE = 128;
char BUFFER[BUF_SIZE] = {0};

void* main_thread(void* arg) {
    int id_client = *static_cast<int*>(arg);
    int count_bytes;
	while(true) {
		memset(BUFFER, 0x00, BUF_SIZE);
        #ifdef _WIN32
        count_bytes = recv(id_client, BUFFER, BUF_SIZE, 0);
        #endif
        #ifdef __linux__
        count_bytes = read(id_client, BUFFER, BUF_SIZE, 0);
        #endif
		if (count_bytes <= 0) {
			printf("[SERVER] Got %d bytes. Something seems to be wrong.\n");
			break;
		}
		else {
			printf("%s\n", BUFFER);
		}
    }
    return 0;
}

