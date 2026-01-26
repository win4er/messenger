#include "server.hpp"

int main(int argc, const char* argv[]) {
	
    // return EXIT_FAILURE;
	Server messenger(
		AF_INET,
		SOCK_STREAM,
		std::atoi(argv[1]),
		256
	);
	messenger.run();

    return EXIT_SUCCESS;
}
