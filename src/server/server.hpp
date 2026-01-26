#ifndef SERVER_HPP
#define SERVER_HPP

#ifdef _WIN32 // to build project for windows systems
#define WIN32_LEAN_AND_MEAN // to avoid conflicts
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

#include <map>          // just to store some BS
#include <string>       // no comments
#include <vector>       // no comments
#include <chrono>       // to write some time-related bullshit
#include <fstream>      // to make/write files

void* recv_thread(void* arg);

// size_t IPv4 = AF_INET;
// size_t IPv6 = AF_INET6;
// size_t TCP = SOCK_STREAM;
// size_t UDP = SOCK_DGRAM;

class Server {

private:
	size_t IPv;
	size_t PROTOCOL;
	size_t PORT;

	size_t ID;
	size_t AMOUNT;
	size_t RES;
		
	int BUF_SIZE;
	char* BUFFER;

public:
	Server(
		size_t IP_v = AF_INET, //IPv4,
		size_t Proto = SOCK_STREAM, //TCP,
		size_t Port = 1,
		size_t BufSize = 0
	);
	~Server();
	friend void* recv_thread(void* arg);
	
	// WARNING: run func is depends on 'recv_thread' func
	int run();
};

#endif
