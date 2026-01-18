#include "server.hpp"

// TODO:
// make good argparser for this project
// refactor this shit, too much lines for single func
void* th1(void* arg) {
    char buf[64];
    int id_client = *static_cast<int*>(arg);
    //here I want wait name of client!!!
    while(true) {
		memset(buf, 0x00, 64);
		int count_bytes = recv(id_client, buf, 64, 0);
		if (count_bytes < 0) {
			std::cerr << "SMTH went WRONG" << std::endl;
			break;
		} 
		else if (count_bytes > 0) {
		    std::cout << buf << std::endl;
        }
    }
    return 0;
}

