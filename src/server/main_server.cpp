#include "server.hpp"

enum CFG {
	IPv4 = AF_INET;
	IPv6 = AF_INET6;
	TCP = SOCK_STREAM;
	UDP = SOCK_DGRAM;
}

const int AMOUNT = 32;

// args: port
int main(int argc, const char* argv[]) {
    printf("Server is up. Listening...\n");
	
	// here i can use 0 in 'protocol' arg
	// as a default protocol,
	// because i already
	// selected TCP as a 'type' arg
	int id_socket = socket(IPv4, TCP, IPPROTO_TCP);
    assert(id_socket > 0);

    sockaddr_in addr;
    addr.sin_family = IPv4;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    int res = bind(id_socket, (sockaddr*)&addr, sizeof(addr));
    assert(res == 0);

    res = listen(id_socket, AMOUNT);

    pthread_t id_thread;
    
    while(1) {
    	int id_client = accept(id_socket, nullptr, nullptr);
    	assert(id_client > 0);
		auto now = std::chrono::steady_clock::now();
		//TIME_CONNECTION_AR.push_back(now);
    	pthread_create(&id_thread, nullptr, th1, &id_client);
		//check_timeout();
    }

    #ifdef _WIN32
    closesocket(id_socket);
    #endif

    #ifdef __linux__
    close(id_socket);
    #endif
    return 0;
}
