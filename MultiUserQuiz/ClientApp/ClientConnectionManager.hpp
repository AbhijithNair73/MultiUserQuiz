// ClientConnectionManager.hpp
#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>
#include "ClientQuizController.hpp"

using client = websocketpp::client<websocketpp::config::asio_tls_client>;
using connection_hdl = websocketpp::connection_hdl;
using json = nlohmann::json;

class ClientConnectionManager {

private:
    std::unique_ptr<ClientQuizController> quiz_controller;
    client ws_client;
    client::connection_ptr connection;
    std::thread client_thread;
    std::atomic<bool> is_running;
    std::atomic<bool> is_connected;

    // TLS configuration
    std::shared_ptr<websocketpp::lib::asio::ssl::context> OnTlsInit (connection_hdl hdl);

    // WebSocket event handlers
    void OnMessage (connection_hdl hdl, client::message_ptr msg);
    void OnOpen (connection_hdl hdl);
    void OnClose (connection_hdl hdl);
    void OnFail (connection_hdl hdl);

    // Message sending
    void SendMessage (const json & message);

public:
    ClientConnectionManager ();
    ~ClientConnectionManager ();

    // Connection management
    bool ConnectToServer (const std::string & uri = "wss://127.0.0.1:9002");
    void Disconnect ();
    bool IsConnected () const;

    // Get controller for UI interaction
    ClientQuizController & GetQuizController ();
};