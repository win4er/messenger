#include "server.hpp"

enum CFG {
	IPv4 = AF_INET,
	IPv6 = AF_INET6,
	TCP = SOCK_STREAM,
	UDP = SOCK_DGRAM,
};

const int AMOUNT = 32;

// args: port
int main(int argc, const char* argv[]) {
    printf("Server is up. Listening...\n");
	
	#ifdef _WIN32
	WSADATA ws = {0};
    int v = WSAStartup(MAKEWORD(2,2), &ws);
	if (v != 0) {
		printf("error initialising winsock: %d\n", v);
        getchar();
        return EXIT_FAILURE;
    }
	#endif


	// here i can use 0 in 'protocol' arg
	// as a default protocol,
	// because i already have selected TCP as a 'type' arg
	int id_socket = socket(CFG::IPv4, CFG::TCP, IPPROTO_TCP);
	printf("%d\n", id_socket);
    assert(id_socket != SOCKET_ERROR);

    sockaddr_in addr;
    addr.sin_family = CFG::IPv4;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    int res = bind(id_socket, (sockaddr*)&addr, sizeof(addr));
    assert(res == 0);

    res = listen(id_socket, AMOUNT);

    pthread_t id_thread;
    int id_client;
    
	// well, actually, i dont like while(true)-like constructions,
	// i MUST fix it later...
    while(true) {
    	id_client = accept(id_socket, nullptr, nullptr);
    	assert(id_client > 0);
    	pthread_create(&id_thread, nullptr, main_thread, &id_client);
    }

    #ifdef _WIN32
    closesocket(id_socket);
    #endif

    #ifdef __linux__
    close(id_socket);
    #endif
    return EXIT_SUCCESS;
}
