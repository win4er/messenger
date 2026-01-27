#include "client.hpp"

Client::Client(
	std::string username,
	std::string address,
	size_t IP_v,
	size_t protocol,
	size_t port,
	size_t buffer_size
) {
    
	this->PORT = port; // number of port
    this->USERNAME = username; // Username
	
	this->BUF_SIZE = buffer_size;
	this->BUFFER = new char[this->BUF_SIZE]; 
    
	this->IPv = IP_v;
	this->SERVER_ADDRESS = address;
	this->PORT = port;
	this->PROTOCOL = protocol;
	
	#ifdef _WIN32
	WSADATA ws = {0};
    int v = WSAStartup(MAKEWORD(2,2), &ws);
	if (v != 0) {
		printf("error initialising winsock: %d\n", v);
        getchar();
        return;
    }
	#endif
	
	this->ID = socket(this->IPv, this->PROTOCOL, 0); // ini the socket i guess
    assert(this->ID > 0); // just for debugging: make sure server is online
	
	// just a config for making a socket connection in case we want get data
    sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(this->SERVER_ADDRESS.c_str());
    addr.sin_port = htons(this->PORT);
    addr.sin_family = this->IPv;
    
	this->RES = connect(this->ID, (sockaddr *)&addr, sizeof(addr));
    assert(this->RES == 0);

	printf("User '%s' has been connected to '%s:%d'\n", this->USERNAME.c_str(), this->SERVER_ADDRESS.c_str(), this->PORT);
}

Client::~Client() {
    #ifdef _WIN32
    closesocket(this->ID);
    #endif

    #ifdef __linux__
    close(this->ID);
    #endif
}

void Client::thread_recv(int id_socket) {
	size_t BUF_SIZE = 256;
	char BUFFER[BUF_SIZE] = {0};

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

Client::run() {
    std::thread th(thread_recv, this->ID);
    th.detach();
    
    std::string message;
   
	// here we are sending the messages to the server
    while (true) {
        message = "";
        std::cin >> message;
		message = "["+this->USERNAME+"]" + message;
        #ifdef _WIN32
        send(this->ID, message.c_str(), message.size() + 1, 0);
        #endif
        #ifdef __linux__
        write(this->ID, message.c_str(), message.size() + 1, 0);
        #endif
    }
    #ifdef _WIN32
    closesocket(this->ID);
    #endif

    #ifdef __linux__
    close(this->ID);
    #endif

    return 0;
}
