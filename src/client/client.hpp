#ifndef CLIENT_HPP
#define CLIENT_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // to avoid conflicts
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // to link needed lib
#endif

#ifdef __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#endif


#include <iostream>
#include <assert.h>
#include <cstring>
#include <thread>

void thread_recv(int id_socket);

#endif
