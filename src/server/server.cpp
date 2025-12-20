#include "server.hpp"

std::vector<int> ID_CLIENT_AR;
std::vector<std::string> NAME_CLIENT_AR;
//std::vector<std::string> TIME_CONNECTION_AR;


bool check_w(std::string word1, std::string word2) {
    if (word1.size() + 1 == word2.size()) {
        for (int i = 0; i < word1.size(); ++i) {
            if (word1[i] != word2[i]) return false;
        }
        return true;
    }
    return false;
}

bool check_id(std::vector<int> vector, int element) {
    for (int i = 0; i < vector.size(); ++i) {
        if (vector[i] == element) return true;
    }
    return false;
}

bool check_el(char* ar, char element) {
    int length = sizeof(ar) / sizeof(char);
    for (int i = 0; i < length; ++i) {
        if (ar[i] == element) return true;
    }
    return false;
}

int get_id(std::vector<std::string> vector, std::string element) {
    for (int i = 0; i < vector.size(); ++i) {
        if (vector[i] == element) return i;
    }
    return -1;
}

int get_id_s(std::vector<int> vector, int element) {
    for (int i = 0; i < vector.size(); ++i) {
        if (vector[i] == element) return i;
    }
    return -1;
}

bool check_name(std::vector<std::string> ar, std::string element) {
    for (int i = 0; i < ar.size(); ++i) {
        if (ar[i] == element) return true;
    }
    return false;
}

std::string get_translate(std::string word) {
    std::ifstream file("enrus.txt");
    std::string result;
    bool flag = false;
    while (std::getline(file, result)) {
		if (check_w(word, result)) {
			file.close();
			return result;
		}
    }
    file.close();
    
    std::ofstream und_file("undefined_words.txt", std::ios_base::app);
    und_file << word;
    und_file.close();
    std::string err_word = "слово не найдено";
    return err_word;

}

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
					std::string story = "from  the moment u became a programmer there's nothing seems to be funny";
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
			else if (check_el(buf, ':') and buf[0] == '@') {
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
				else send(ID_CLIENT_AR[id_chat], message.c_str(), message.size() + 1, 0);
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
    return 0;
}


