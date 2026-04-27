#ifndef PEER_HPP
#define PEER_HPP

#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <memory>
#include <netinet/in.h>

class Peer {
public:
    struct PeerInfo {
        std::string address;
        uint16_t port;
        std::string id;
        int socket = -1;
        
        bool operator==(const PeerInfo& other) const {
            return id == other.id;
        }
    };

    using MessageCallback = std::function<void(const std::string& from_id, 
                                                const std::string& message)>;
    using PeerCallback = std::function<void(const PeerInfo& peer)>;
    using ErrorCallback = std::function<void(const std::string& error)>;

    Peer(
        const std::string& listen_address = "0.0.0.0",
        uint16_t listen_port = 0,
        const std::string& node_id = "",
        size_t buffer_size = 65536
    );
    
    ~Peer();

    bool start();
    void stop();
    
    bool connect_to_peer(const std::string& address, uint16_t port);
    void disconnect_from_peer(const std::string& peer_id);
    
    bool send_to_peer(const std::string& peer_id, const std::string& message);
    bool broadcast(const std::string& message, const std::string& exclude_id = "");
    
    PeerInfo get_self_info() const;
    std::vector<PeerInfo> get_connected_peers() const;
    bool is_running() const;
    std::string get_node_id() const;

    void on_message_received(MessageCallback callback);
    void on_peer_connected(PeerCallback callback);
    void on_peer_disconnected(PeerCallback callback);
    void on_error(ErrorCallback callback);

private:
    struct Impl {
        int listen_socket = -1;
        uint16_t listen_port = 0;
        std::string listen_address;
        std::string node_id;
        
        std::vector<PeerInfo> connected_peers;
        std::mutex peers_mutex;
        
        std::atomic<bool> running{false};
        
        MessageCallback message_callback;
        PeerCallback connected_callback;
        PeerCallback disconnected_callback;
        ErrorCallback error_callback;
        
        size_t buffer_size;
        std::unique_ptr<char[]> read_buffer;
        
        std::atomic<bool> stop_threads{false};
        
        std::string safe_receive(int sock);
        bool safe_send(int sock, const std::string& message);
    };
    
    std::unique_ptr<Impl> impl_;
    
    void accept_loop(); 
    void receive_loop(int peer_socket, std::string peer_id); 
    
    void notify_error(const std::string& error);
    std::string generate_id() const;
};

#endif
