#include "client.hpp"

int main(int argc, char** argv) {
    
    std::cout << "simple client ver 1.0...." << std::endl;
    
    int sin_port = std::stoi(argv[1]);
    std::string name = argv[2];

    int id_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(id_socket > 0);

    sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    std::cout << "127.0.0.1  " << inet_addr("127.0.0.1") << std::endl;
    addr.sin_port = htons(sin_port);
    addr.sin_family = AF_INET;

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
    
    char buf[64];
    std::string message;
    
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

