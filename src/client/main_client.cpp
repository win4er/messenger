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
	
	Client user(
		name,
		ADDRESS,
		AF_INET,
		SOCK_STREAM,
		sin_port,
		256
	);
	user.run();
    return 0;
}

