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

void thread_recv(int id_socket);

#endif
