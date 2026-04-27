// src/main.cpp
#include "peer.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <signal.h>
#include <atomic>
#include <termios.h>
#include <unistd.h>

std::atomic<bool> quit_flag(false);
std::atomic<bool> peer_stopped(false);

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[INFO] Shutting down..." << std::endl;
        quit_flag = true;
    }
}

struct TerminalSettings {
    struct termios old_settings;
    
    void enable_raw_mode() {
        tcgetattr(STDIN_FILENO, &old_settings);
        struct termios new_settings = old_settings;
        new_settings.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    }
    
    void restore() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    }
};

struct Config {
    std::string listen_address = "0.0.0.0";
    uint16_t listen_port = 0;
    std::string node_id;
    
    bool parse(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--help" || arg == "-h") {
                print_help();
                return false;
            }
            else if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
                listen_port = static_cast<uint16_t>(std::stoi(argv[++i]));
            }
            else if ((arg == "--address" || arg == "-a") && i + 1 < argc) {
                listen_address = argv[++i];
            }
            else if ((arg == "--id" || arg == "-i") && i + 1 < argc) {
                node_id = argv[++i];
            }
        }
        return true;
    }
    
    void print_help() const {
        std::cout << R"(
P2P Chat - Decentralized Messenger

Usage: p2p_chat [OPTIONS]

Options:
  -h, --help          Show this help message
  -p, --port PORT     Port to listen on (0 = random, default: 0)
  -a, --address ADDR  Address to bind (default: 0.0.0.0)
  -i, --id ID         Node identifier (default: random)

Commands in chat:
  /connect IP:PORT     Connect to a peer
  /c IP:PORT          Short form of connect
  /list               Show connected peers
  /l                  Short form of list
  /broadcast MESSAGE   Send to all peers
  /b MESSAGE          Short form of broadcast
  /quit               Exit the program
  /q                  Short form of quit

Examples:
  ./p2p_chat -p 8000
  /connect 127.0.0.1:8001
)" << std::endl;
    }
};

int main(int argc, char** argv) {
    Config config;
    if (!config.parse(argc, argv)) {
        return 0;
    }
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    Peer peer(config.listen_address, config.listen_port, config.node_id);
    
    peer.on_message_received([](const std::string& from_id, const std::string& message) {
        std::cout << "\r\033[K[" << from_id.substr(0, 8) << "] " 
                  << message << std::endl;
        std::cout << "> " << std::flush;
    });
    
    peer.on_peer_connected([](const Peer::PeerInfo& p) {
        std::cout << "\r\033[K[CONNECTED] " << p.id.substr(0, 8) 
                  << " (" << p.address << ":" << p.port << ")" << std::endl;
        std::cout << "> " << std::flush;
    });
    
    peer.on_peer_disconnected([](const Peer::PeerInfo& p) {
        std::cout << "\r\033[K[DISCONNECTED] " << p.id.substr(0, 8) << std::endl;
        std::cout << "> " << std::flush;
    });
    
    peer.on_error([](const std::string& error) {
        std::cout << "\r\033[K[ERROR] " << error << std::endl;
        std::cout << "> " << std::flush;
    });
    
    if (!peer.start()) {
        std::cerr << "Failed to start peer" << std::endl;
        return 1;
    }
    
    auto self = peer.get_self_info();
    std::cout << "\nP2P Chat Started!" << std::endl;
    std::cout << "Node ID: " << self.id << std::endl;
    std::cout << "Port: " << self.port << std::endl;
    std::cout << "Type /help for commands\n" << std::endl;
    
    TerminalSettings terminal;
    terminal.enable_raw_mode();
    
    std::string input_buffer;
    char c;
    
    std::cout << "> " << std::flush;
    
    while (!quit_flag) {
        ssize_t bytes_read = read(STDIN_FILENO, &c, 1);
        
        if (bytes_read > 0) {
            if (c == '\n' || c == '\r') {
                std::cout << std::endl;
                
                if (input_buffer.empty()) {
                }
                else if (input_buffer == "/quit" || input_buffer == "/q") {
                    std::cout << "[INFO] Shutting down..." << std::endl;
                    quit_flag = true;
                }
                else if (input_buffer == "/help" || input_buffer == "/h") {
                    std::cout << "\nCommands:\n"
                              << "  /connect IP:PORT    Connect to peer\n"
                              << "  /c IP:PORT          Short form of connect\n"
                              << "  /list               Show connected peers\n"
                              << "  /l                  Short form of list\n"
                              << "  /broadcast MSG      Send to all peers\n"
                              << "  /b MSG              Short form of broadcast\n"
                              << "  /quit               Exit the program\n"
                              << "  /q                  Short form of quit\n\n";
                }
                else if (input_buffer == "/list" || input_buffer == "/l") {
                    auto peers = peer.get_connected_peers();
                    std::cout << "\nConnected peers (" << peers.size() << "):\n";
                    for (const auto& p : peers) {
                        std::cout << "  " << p.id.substr(0, 8) 
                                  << " @ " << p.address << ":" << p.port << "\n";
                    }
                    std::cout << std::endl;
                }
                else if (input_buffer.substr(0, 9) == "/connect ") {
                    std::string address_str = input_buffer.substr(9);
                    auto colon_pos = address_str.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string ip = address_str.substr(0, colon_pos);
                        std::string port_str = address_str.substr(colon_pos + 1);
                        try {
                            uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
                            peer.connect_to_peer(ip, port);
                        } catch (...) {
                            std::cout << "Invalid port number. Use: /connect IP:PORT\n";
                        }
                    } else {
                        std::cout << "Invalid format. Use: /connect IP:PORT\n";
                    }
                }
                else if (input_buffer.substr(0, 3) == "/c ") {
                    std::string address_str = input_buffer.substr(3);
                    auto colon_pos = address_str.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string ip = address_str.substr(0, colon_pos);
                        std::string port_str = address_str.substr(colon_pos + 1);
                        try {
                            uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
                            peer.connect_to_peer(ip, port);
                        } catch (...) {
                            std::cout << "Invalid port number. Use: /c IP:PORT\n";
                        }
                    } else {
                        std::cout << "Invalid format. Use: /c IP:PORT\n";
                    }
                }
                else if (input_buffer.substr(0, 11) == "/broadcast ") {
                    std::string message = input_buffer.substr(11);
                    if (!message.empty()) {
                        peer.broadcast(message);
                    }
                }
                else if (input_buffer.substr(0, 3) == "/b ") {
                    std::string message = input_buffer.substr(3);
                    if (!message.empty()) {
                        peer.broadcast(message);
                    }
                }
                else if (input_buffer.substr(0, 5) == "/msg ") {
                    std::string rest = input_buffer.substr(5);
                    auto first_space = rest.find(' ');
                    if (first_space != std::string::npos) {
                        std::string peer_id = rest.substr(0, first_space);
                        std::string message = rest.substr(first_space + 1);
                        if (!peer_id.empty() && !message.empty()) {
                            peer.send_to_peer(peer_id, message);
                        } else {
                            std::cout << "Use: /msg PEER_ID MESSAGE\n";
                        }
                    } else {
                        std::cout << "Use: /msg PEER_ID MESSAGE\n";
                    }
                }
                else {
                    peer.broadcast(input_buffer);
                }
                
                input_buffer.clear();
                
                if (!quit_flag) {
                    std::cout << "> " << std::flush;
                }
            }
            else if (c == 127 || c == 8) {
                if (!input_buffer.empty()) {
                    input_buffer.pop_back();
                    std::cout << "\b \b" << std::flush;
                }
            }
            else if (c == 3) {
                std::cout << "\n[INFO] Shutting down..." << std::endl;
                quit_flag = true;
            }
            else if (c >= 32) {
                input_buffer += c;
                std::cout << c << std::flush;
            }
        }
        else if (bytes_read == -1 && errno != EAGAIN) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    terminal.restore();
    
    peer.stop();
    peer_stopped = true;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\nGoodbye!\n";
    return 0;
}
