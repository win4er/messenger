#include "client.hpp"

enum CFG {
	IPv4 = AF_INET,
	IPv6 = AF_INET6,
	TCP = SOCK_STREAM,
	UDP = SOCK_DGRAM,
};

const char* ADDRESS = "127.0.0.1";


int main(int argc, char** argv) {
    int sin_port = std::stoi(argv[1]); // number of port
    std::string name = argv[2]; // Username
	
	#ifdef _WIN32
	WSADATA ws = {0};
    int v = WSAStartup(MAKEWORD(2,2), &ws);
	if (v != 0) {
		printf("error initialising winsock: %d\n", v);
        getchar();
        return 1;
    }
	#endif
    
	int id_socket = socket(CFG::IPv4, CFG::TCP, IPPROTO_TCP); // ini the socket i guess
    assert(id_socket > 0); // just for debugging: make sure server is online
	
	// just a config for making a socket connection in case we want get data
    sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(ADDRESS);
    std::cout << ADDRESS << "  " << inet_addr(ADDRESS) << std::endl;
    addr.sin_port = htons(sin_port);
    addr.sin_family = CFG::IPv4;

	// here we see a connection, res = count of received bytes i guess
    int res = connect(id_socket, (sockaddr *)&addr, sizeof(addr));
    assert(res == 0);
    #ifdef _WIN32
    send(id_socket, name.c_str(), name.size() + 1, 0);
    #endif
    #ifdef __linux__
    write(id_socket, name.c_str(), name.size() + 1, 0);
    #endif
    
    std::thread th(thread_recv, id_socket);
    th.detach();
    
    std::string message;
   
	// here we are sending the messages to the server
    while (true) {
        message = "";
        std::cin >> message;
        #ifdef _WIN32
        send(id_socket, message.c_str(), message.size() + 1, 0);
        #endif
        #ifdef __linux__
        write(id_socket, message.c_str(), message.size() + 1, 0);
        #endif
    }
    #ifdef _WIN32
    closesocket(id_socket);
    #endif

    #ifdef __linux__
    close(id_socket);
    #endif

    return 0;
}

