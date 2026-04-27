#include "peer.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <random>
#include <algorithm>


Peer::Peer(
    const std::string& listen_address,
    uint16_t listen_port,
    const std::string& node_id,
    size_t buffer_size
) : impl_(std::make_unique<Impl>()) {
    
    impl_->listen_address = listen_address;
    impl_->listen_port = listen_port;
    impl_->node_id = node_id.empty() ? generate_id() : node_id;
    impl_->buffer_size = buffer_size;
    impl_->read_buffer = std::make_unique<char[]>(buffer_size);
}

Peer::~Peer() {
    stop();
}


std::string Peer::generate_id() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i) {
        ss << dis(gen);
    }
    return ss.str();
}


bool Peer::start() {
    if (impl_->running) {
        notify_error("Peer already running");
        return false;
    }
    
    impl_->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (impl_->listen_socket < 0) {
        notify_error("Failed to create socket: " + std::string(strerror(errno)));
        return false;
    }
    
    int opt = 1;
    if (setsockopt(impl_->listen_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                   &opt, sizeof(opt)) < 0) {
        setsockopt(impl_->listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(impl_->listen_port);
    
    if (impl_->listen_address == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, impl_->listen_address.c_str(), &addr.sin_addr);
    }
    
    if (bind(impl_->listen_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        notify_error("Failed to bind: " + std::string(strerror(errno)));
        close(impl_->listen_socket);
        return false;
    }
    
    socklen_t addr_len = sizeof(addr);
    getsockname(impl_->listen_socket, (sockaddr*)&addr, &addr_len);
    impl_->listen_port = ntohs(addr.sin_port);
    
    if (listen(impl_->listen_socket, SOMAXCONN) < 0) {
        notify_error("Failed to listen: " + std::string(strerror(errno)));
        close(impl_->listen_socket);
        return false;
    }
    
    impl_->running = true;
    impl_->stop_threads = false;
    
    std::thread(&Peer::accept_loop, this).detach();
    
    std::cout << "[PEER] Started. ID: " << impl_->node_id 
              << " Listening on port: " << impl_->listen_port << std::endl;
    
    return true;
}

void Peer::stop() {
    if (!impl_->running) return;
    
    impl_->running = false;
    impl_->stop_threads = true;
    
    if (impl_->listen_socket >= 0) {
        shutdown(impl_->listen_socket, SHUT_RDWR);
        close(impl_->listen_socket);
        impl_->listen_socket = -1;
    }
    
    {
        std::lock_guard<std::mutex> lock(impl_->peers_mutex);
        for (auto& peer : impl_->connected_peers) {
            if (peer.socket >= 0) {
                shutdown(peer.socket, SHUT_RDWR);
                close(peer.socket);
            }
        }
        impl_->connected_peers.clear();
    }
    
    std::cout << "[PEER] Stopped." << std::endl;
}


bool Peer::connect_to_peer(const std::string& address, uint16_t port) {
    if (!impl_->running) {
        notify_error("Peer not running");
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(impl_->peers_mutex);
        for (auto& p : impl_->connected_peers) {
            if (p.address == address && p.port == port) {
                notify_error("Already connected to " + address + ":" + std::to_string(port));
                return false;
            }
        }
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        notify_error("Failed to create socket: " + std::string(strerror(errno)));
        return false;
    }
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
        notify_error("Invalid address: " + address);
        close(sock);
        return false;
    }
    
    std::cout << "[PEER] Connecting to " << address << ":" << port << "..." << std::endl;
    
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        notify_error("Failed to connect: " + std::string(strerror(errno)));
        close(sock);
        return false;
    }
    
    std::string handshake = "HELLO:" + impl_->node_id + ":" + 
                           std::to_string(impl_->listen_port);
    if (!impl_->safe_send(sock, handshake)) {
        notify_error("Failed to send handshake");
        close(sock);
        return false;
    }
    
    std::string response = impl_->safe_receive(sock);
    if (response.empty() || response.substr(0, 6) != "HELLO:") {
        notify_error("Invalid handshake response");
        close(sock);
        return false;
    }
    
    auto first_colon = response.find(':');
    auto second_colon = response.find(':', first_colon + 1);
    std::string peer_id = response.substr(first_colon + 1, second_colon - first_colon - 1);
    uint16_t peer_port = std::stoi(response.substr(second_colon + 1));
    
    PeerInfo info;
    info.address = address;
    info.port = peer_port;
    info.id = peer_id;
    info.socket = sock;
    
    {
        std::lock_guard<std::mutex> lock(impl_->peers_mutex);
        impl_->connected_peers.push_back(info);
    }
    
    std::cout << "[PEER] Connected to " << peer_id << " at " 
              << address << ":" << port << std::endl;
    
    if (impl_->connected_callback) {
        impl_->connected_callback(info);
    }
    
    std::thread(&Peer::receive_loop, this, sock, peer_id).detach();
    
    return true;
}

void Peer::disconnect_from_peer(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(impl_->peers_mutex);
    
    auto it = std::find_if(impl_->connected_peers.begin(), 
                          impl_->connected_peers.end(),
                          [&peer_id](const PeerInfo& p) { return p.id == peer_id; });
    
    if (it != impl_->connected_peers.end()) {
        if (it->socket >= 0) {
            shutdown(it->socket, SHUT_RDWR);
            close(it->socket);
        }
        
        if (impl_->disconnected_callback) {
            impl_->disconnected_callback(*it);
        }
        
        impl_->connected_peers.erase(it);
        std::cout << "[PEER] Disconnected from " << peer_id << std::endl;
    }
}


bool Peer::send_to_peer(const std::string& peer_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(impl_->peers_mutex);
    
    auto it = std::find_if(impl_->connected_peers.begin(), 
                          impl_->connected_peers.end(),
                          [&peer_id](const PeerInfo& p) { return p.id == peer_id; });
    
    if (it == impl_->connected_peers.end()) {
        notify_error("Peer not found: " + peer_id);
        return false;
    }
    
    std::string packet = "MSG:" + impl_->node_id + ":" + message;
    return impl_->safe_send(it->socket, packet);
}

bool Peer::broadcast(const std::string& message, const std::string& exclude_id) {
    bool success = true;
    std::vector<PeerInfo> peers_copy;
    
    {
        std::lock_guard<std::mutex> lock(impl_->peers_mutex);
        peers_copy = impl_->connected_peers;
    }
    
    for (auto& peer : peers_copy) {
        if (peer.id != exclude_id) {
            if (!send_to_peer(peer.id, message)) {
                success = false;
            }
        }
    }
    
    return success;
}


void Peer::on_message_received(MessageCallback callback) {
    impl_->message_callback = std::move(callback);
}

void Peer::on_peer_connected(PeerCallback callback) {
    impl_->connected_callback = std::move(callback);
}

void Peer::on_peer_disconnected(PeerCallback callback) {
    impl_->disconnected_callback = std::move(callback);
}

void Peer::on_error(ErrorCallback callback) {
    impl_->error_callback = std::move(callback);
}


Peer::PeerInfo Peer::get_self_info() const {
    PeerInfo info;
    info.address = impl_->listen_address;
    info.port = impl_->listen_port;
    info.id = impl_->node_id;
    return info;
}

std::vector<Peer::PeerInfo> Peer::get_connected_peers() const {
    std::lock_guard<std::mutex> lock(impl_->peers_mutex);
    return impl_->connected_peers;
}

bool Peer::is_running() const {
    return impl_->running;
}

std::string Peer::get_node_id() const {
    return impl_->node_id;
}


void Peer::accept_loop() {
    while (!impl_->stop_threads) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_sock = accept(impl_->listen_socket, 
                                (sockaddr*)&client_addr, &client_len);
        
        if (client_sock < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            if (!impl_->stop_threads) {
                notify_error("Accept failed: " + std::string(strerror(errno)));
            }
            break;
        }
        
        std::string handshake = impl_->safe_receive(client_sock);
        if (handshake.empty() || handshake.substr(0, 6) != "HELLO:") {
            notify_error("Invalid handshake from incoming connection");
            close(client_sock);
            continue;
        }
        
        auto first_colon = handshake.find(':');
        auto second_colon = handshake.find(':', first_colon + 1);
        std::string peer_id = handshake.substr(first_colon + 1, 
                                              second_colon - first_colon - 1);
        uint16_t peer_port = std::stoi(handshake.substr(second_colon + 1));
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        std::string response = "HELLO:" + impl_->node_id + ":" + 
                              std::to_string(impl_->listen_port);
        if (!impl_->safe_send(client_sock, response)) {
            notify_error("Failed to send handshake response");
            close(client_sock);
            continue;
        }
        
        PeerInfo info;
        info.address = std::string(client_ip);
        info.port = peer_port;
        info.id = peer_id;
        info.socket = client_sock;
        
        {
            std::lock_guard<std::mutex> lock(impl_->peers_mutex);
            impl_->connected_peers.push_back(info);
        }
        
        std::cout << "[PEER] New connection from " << peer_id 
                  << " (" << client_ip << ":" << peer_port << ")" << std::endl;
        
        if (impl_->connected_callback) {
            impl_->connected_callback(info);
        }
        
        std::thread(&Peer::receive_loop, this, client_sock, peer_id).detach();
    }
}

void Peer::receive_loop(int peer_socket, std::string peer_id) {
    while (!impl_->stop_threads) {
        std::string data = impl_->safe_receive(peer_socket);
        
        if (data.empty()) {
            disconnect_from_peer(peer_id);
            break;
        }
        
        if (data.substr(0, 4) == "MSG:") {
            auto first_colon = data.find(':');
            auto second_colon = data.find(':', first_colon + 1);
            
            if (first_colon != std::string::npos && second_colon != std::string::npos) {
                std::string from_id = data.substr(first_colon + 1, 
                                                 second_colon - first_colon - 1);
                std::string content = data.substr(second_colon + 1);
                
                if (impl_->message_callback) {
                    impl_->message_callback(from_id, content);
                }
            }
        }
        else if (data == "PING") {
            impl_->safe_send(peer_socket, "PONG");
        }
    }
}


std::string Peer::Impl::safe_receive(int sock) {
    uint32_t msg_len_net;
    ssize_t bytes_read = recv(sock, &msg_len_net, sizeof(msg_len_net), MSG_WAITALL);
    
    if (bytes_read <= 0) {
        return ""; 
    }
    
    uint32_t msg_len = ntohl(msg_len_net);
    
    if (msg_len == 0 || msg_len > buffer_size) {
        return ""; 
    }
    
    bytes_read = recv(sock, read_buffer.get(), msg_len, MSG_WAITALL);
    
    if (bytes_read <= 0) {
        return "";
    }
    
    return std::string(read_buffer.get(), bytes_read);
}

bool Peer::Impl::safe_send(int sock, const std::string& message) {
    uint32_t msg_len = message.size();
    uint32_t msg_len_net = htonl(msg_len);
    
    ssize_t sent = send(sock, &msg_len_net, sizeof(msg_len_net), MSG_NOSIGNAL);
    if (sent != sizeof(msg_len_net)) {
        return false;
    }
    
    sent = send(sock, message.c_str(), msg_len, MSG_NOSIGNAL);
    return sent == static_cast<ssize_t>(msg_len);
}

void Peer::notify_error(const std::string& error) {
    std::cerr << "[PEER ERROR] " << error << std::endl;
    if (impl_->error_callback) {
        impl_->error_callback(error);
    }
}
