#include <assert.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>


std::vector<int> ID_CLIENT_AR;
std::vector<std::string> NAME_CLIENT_AR;
//std::vector<std::string> TIME_CONNECTION_AR;


bool check_w(std::string word1, std::string word2) {
    if (word1.size() + 1 == word2.size()) {
	for (int i = 0; i < word1.size(); ++i) {
	    if (word1[i] != word2[i]) {
		return false;
	    }
	}
	return true;
    }
    return false;
}

bool check_id(std::vector<int> vector, int element) {
    for (int i = 0; i < vector.size(); ++i) {
	if (vector[i] == element) {
	    return true;
	}
    }
    return false;
}

bool check_el(char* ar, char element) {
    int length = sizeof(ar) / sizeof(char);
    for (int i = 0; i < length; ++i) {
	if (ar[i] == element) {
	    return true;
	}
    }
    return false;
}

int get_id(std::vector<std::string> vector, std::string element) {
    for (int i = 0; i < vector.size(); ++i) {
	if (vector[i] == element) {
	    return i;
	}
    }
    return -1;
}

int get_id_s(std::vector<int> vector, int element) {
    for (int i = 0; i < vector.size(); ++i) {
	if (vector[i] == element) {
	    return i;
	}
    }
    return -1;
}

bool check_name(std::vector<std::string> ar, std::string element) {
    for (int i = 0; i < ar.size(); ++i) {
	if (ar[i] == element) {
	    return true;
	}
    }
    return false;
}

std::string get_translate(std::string word) {
    std::ifstream file("ENRUS.TXT");
    std::string result;
    bool flag = false;
    while (std::getline(file, result)) {
	if (flag) {
	    file.close();
	    return result;
	}
	if (check_w(word, result)) {
	    flag = true;
	}
    }
    file.close();
    
    std::ofstream und_file("undefined_words.txt", std::ios_base::app);
    und_file << word;
    und_file.close();
    std::string err_word = "слово не найдено";
    return err_word;

}


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
	    if (buf[0] == '-' and buf[1] == '-') {
	    
		std::string info = buf;
		if (info == "--list") {
		    for (int i = 0; i < NAME_CLIENT_AR.size(); ++i) {
			std::string next = "\n";
		        send(id_client, NAME_CLIENT_AR[i].c_str(), NAME_CLIENT_AR[i].size() + 1, 0);
			send(id_client, next.c_str(), next.size() + 1, 0);
		    }
		} 
		else if (info == "--elapsedtime") {
		}
		else if (info == "--story") {
		    std::string story = "from  the moment u became a progger there's nothing to be funny";
		    send(id_client, story.c_str(), story.size() + 1, 0);		    
		}
		else {
		    char* token = std::strtok(buf, "|");
		    std::string command = token;
		    if (command == "--word") {
			token = std::strtok(NULL, "|");
			std::string word = token;
			std::string tr_word = get_translate(word);
			send(id_client, tr_word.c_str(), tr_word.size() + 1, 0);	    
		    }
		}
		continue;
	    }
	    if (check_id(ID_CLIENT_AR, id_client) == false) {
		std::string name1 = "@";
		std::string name2 = buf;
		std::string name = name1 + name2;
		if (check_name(NAME_CLIENT_AR, name) == false) {
		    ID_CLIENT_AR.push_back(id_client);
		    NAME_CLIENT_AR.push_back(name);
		}
		else {
		    std::string error_dname1 = "[SERVER] NAME IS DUBLICATED : ";
		    std::string error_dname = error_dname1 + name2;
		    send(id_client, error_dname.c_str(), error_dname.size() + 1, 0);
		    break;
		}
	    }
	    else {
		if (check_el(buf, ':') and buf[0] == '@') {
		    char* token = std::strtok(buf, ":");
		    std::string name = token;
		    token = std::strtok(NULL, ":");
		    std::string message = token;
		    int id_chat = get_id(NAME_CLIENT_AR, name);
		    if (id_chat == -1) {
			std::string error_no_name1 = "[SERVER] NO NAME : ";
			std::string error_no_name = error_no_name1 + name;
			send(id_client, error_no_name.c_str(), error_no_name.size() + 1, 0);
		    }
		    else {
			send(ID_CLIENT_AR[id_chat], message.c_str(), message.size() + 1, 0);
		    }
		}
		else {
		    for (int i = 0; i < ID_CLIENT_AR.size(); ++i) {
			if (ID_CLIENT_AR[i] != id_client) {
			    send(ID_CLIENT_AR[i], buf, 64, 0);
			}
		    }
		}
	    }

	}
    }
    
    return 0;
}


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

    close(id_socket);
    return 0;
}
