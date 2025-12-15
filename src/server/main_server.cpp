#include "server.hpp"

int main(int argc, const char* argv[]) {
    std::cout << "my not full server... v3.0 not to much secret " << std::endl;
    
    int id_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    assert(id_socket > 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    int res = bind(id_socket, (sockaddr*)&addr, sizeof(addr));
    assert(res == 0);

    res = listen(id_socket, 32);

    pthread_t id_thread;
    
    while(1) {
    	int id_client = accept(id_socket, nullptr, nullptr);
    	assert(id_client > 0);
	auto now = std::chrono::steady_clock::now();
	//TIME_CONNECTION_AR.push_back(now);
    	pthread_create(&id_thread, nullptr, th1, &id_client);
	//check_timeout();
    }

    #ifdef _WIN32
    closesocket(id_socket);
    #endif

    #ifdef __linux__
    close(id_socket);
    #endif
    return 0;
}
