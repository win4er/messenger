#include "server.hpp"

// TODO:
// 1. make good argparser for this project
// 2. rebuild this shit, i dont like singletone(not essential)
// 3. make communication between clients (p2p)
// 4. make cipher-protocol
// 5. make first tui->gui

void* Server::recv_thread(void* arg) {
	size_t BUF_SIZE = 256;
    char BUFFER[BUF_SIZE] = {0};

	int socket = *static_cast<int*>(arg);
    int count_bytes;
	
	// Avoid infinity loops in next updates.
	while(true) {
		memset(BUFFER, 0x00, BUF_SIZE);
        #ifdef _WIN32
        count_bytes = recv(socket, BUFFER, BUF_SIZE, 0);
        #endif
        #ifdef __linux__
        count_bytes = read(socket, BUFFER, BUF_SIZE, 0);
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

Server::Server(size_t IP_v, size_t Proto, size_t Port, size_t BufSize) {
	// Server variables initialization:	
	this->IPv = IP_v;
	this->PROTOCOL = Proto;
	this->PORT = Port;
		
	this->BUF_SIZE = BufSize;
	this->BUFFER = new char[this->BUF_SIZE];
    
	#ifdef _WIN32
	WSADATA ws = {0};
    int v = WSAStartup(MAKEWORD(2,2), &ws);
	if (v != 0) {
		printf("error initialising winsock: %d\n", v);
        getchar();
        return;
    }
	#endif

	this->ID = socket(this->IPv, this->PROTOCOL, 0);
	printf("%d\n", this->ID);
	
	assert(this->ID != SOCKET_ERROR);
	
    sockaddr_in addr;
    addr.sin_family = this->IPv;
    addr.sin_port = htons(this->PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
	
    this->RES = bind(this->ID, (sockaddr*)&addr, sizeof(addr));
    assert(RES == 0);
    this->RES = listen(this->ID, this->AMOUNT);

	printf("Server is UP...\n");
}

Server::~Server() {
	delete [] this->BUFFER;
}

int Server::run() {
    pthread_t id_thread;
    int id_client;
    
	// well, actually, I dont like while(true)-like constructions,
	// I MUST fix it later...
    while(true) {
    	id_client = accept(this->ID, nullptr, nullptr);
    	assert(this->ID > 0);
    	pthread_create(&id_thread, nullptr, recv_thread, &id_client);
    }

    #ifdef _WIN32
    closesocket(this->ID);
    #endif

    #ifdef __linux__
    close(this->ID);
    #endif
	printf("Server is DOWN.\n");
    return EXIT_SUCCESS;
}

