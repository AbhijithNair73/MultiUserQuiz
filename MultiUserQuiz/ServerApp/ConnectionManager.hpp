#pragma once
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "QuizController.hpp"

using server = websocketpp::server<websocketpp::config::asio_tls>;
using connection_hdl = websocketpp::connection_hdl;

class ConnectionManager {

private:
    std::unique_ptr<QuizController> quiz_controller;
    server ws_server;
    std::vector<std::thread> thread_pool;

    // TLS configuration
    std::shared_ptr<asio::ssl::context> OnTlsInit (connection_hdl hdl);

    // WebSocket event handlers
    void OnMessage (server * s, connection_hdl hdl, server::message_ptr msg);
    void OnOpen (server * s, connection_hdl hdl);
    void OnClose (server * s, connection_hdl hdl);

public:
    ConnectionManager ();
    ~ConnectionManager ();

    void StartServer (int port = 9002);
    void StopServer ();
};
