#ifndef CLIENT_HPP
#define CLIENT_HPP

#ifdef _WIN32 // to build project for windows systems
#define WIN32_LEAN_AND_MEAN        // to avoid conflicts
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // to link needed lib
#endif

#ifdef __linux__        // to build project for linux systems
#include <arpa/inet.h>  // for Unix-like systems
#include <netinet/in.h> // for Unix-like systems
#include <sys/socket.h> // bind(), connect(), listen(), accept()
#include <unistd.h>
#endif

#include <sys/types.h>  // bind(), connect(), listen(), accept()

#include <iostream>     // basic I/O lib
#include <assert.h>     // just for DEBUGGING i think
#include <cstring>
#include <thread>       // to make another thread, i need async programming later

// size_t IPv4 = AF_INET;
// size_t IPv6 = AF_INET6;
// size_t TCP = SOCK_STREAM;
// size_t UDP = SOCK_DGRAM;

class Client {
private:
	size_t ID;
	std::string USERNAME;
	size_t RES;
	
	size_t IPv;
	size_t PROTOCOL;
	std::string SERVER_ADDRESS;
	size_t PORT;
	
	size_t BUF_SIZE;
	char* BUFFER;
public:
	Client(
		std::string username = "User",
		std::string address = "127.0.0.1",
		size_t IP_v = AF_INET,
		size_t protocol = SOCK_STREAM,
		size_t port = 1,
		size_t buffer_size = 256
	);
	~Client();
	int run();
	static void thread_recv(int id_socket);
};

#endif
