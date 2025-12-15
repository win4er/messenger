#ifndef SERVER_HPP
#define SERVER_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // to avoid conflicts
#include <winsock2.h>
#endif

#ifdef __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <sys/types.h>

#include <iostream>
#include <assert.h>
#include <cstring>
#include <thread>

#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>

bool check_w(std::string word1, std::string word2);
bool check_id(std::vector<int> vector, int element);
bool check_el(char* ar, char element);
int get_id(std::vector<std::string> vector, std::string element);
int get_id_s(std::vector<int> vector, int element);
bool check_name(std::vector<std::string> ar, std::string element);
std::string get_translate(std::string word);
void* th1(void* arg);

#endif
