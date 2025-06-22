// ClientConnectionManager.cpp
#include "ClientConnectionManager.hpp"
#include <iostream>

ClientConnectionManager::ClientConnectionManager ()
    : quiz_controller (std::make_unique<ClientQuizController> ()),
    is_running (false),
    is_connected (false)
{

    // Set up quiz controller callback to send messages
    quiz_controller->SetSendMessageCallback ([this] (const json & msg) {
        SendMessage (msg);
    });
}

ClientConnectionManager::~ClientConnectionManager ()
{
    Disconnect ();
}

std::shared_ptr<websocketpp::lib::asio::ssl::context>
ClientConnectionManager::OnTlsInit (connection_hdl hdl)
{
    auto ctx = std::make_shared<websocketpp::lib::asio::ssl::context> (
        websocketpp::lib::asio::ssl::context::tlsv12_client);
    try {
        ctx->set_verify_mode (websocketpp::lib::asio::ssl::verify_none);
        std::cout << "TLS init succeeded." << std::endl;
    } catch (std::exception & e) {
        std::cerr << "TLS init failed: " << e.what () << std::endl;
    }
    return ctx;
}

void ClientConnectionManager::OnMessage (connection_hdl hdl, client::message_ptr msg)
{
    try {
        std::cout << "[RECEIVED] " << msg->get_payload () << std::endl;

        json response = json::parse (msg->get_payload ());
        quiz_controller->ProcessResponse (response);

    } catch (const std::exception & e) {
        std::cerr << "Message processing exception: " << e.what () << std::endl;
    }
}

void ClientConnectionManager::OnOpen (connection_hdl hdl)
{
    std::cout << "[CONNECTED] Successfully connected to server" << std::endl;
    is_connected = true;
    ClientSessionManager::GetInstance ().SetState (ClientState::CONNECTED);
}

void ClientConnectionManager::OnClose (connection_hdl hdl)
{
    std::cout << "[DISCONNECTED] Connection closed" << std::endl;
    is_connected = false;
    ClientSessionManager::GetInstance ().SetState (ClientState::DISCONNECTED);
}

void ClientConnectionManager::OnFail (connection_hdl hdl)
{
    std::cout << "[FAILED] Connection failed" << std::endl;
    is_connected = false;
    ClientSessionManager::GetInstance ().SetState (ClientState::DISCONNECTED);
}

void ClientConnectionManager::SendMessage (const json & message)
{
    if (is_connected && connection) {
        try {
            std::string msg_str = message.dump ();
            std::cout << "[SENDING] " << msg_str << std::endl;
            connection->send (msg_str);
        } catch (const std::exception & e) {
            std::cerr << "Send message exception: " << e.what () << std::endl;
        }
    } else {
        std::cerr << "Cannot send message: not connected" << std::endl;
    }
}

bool ClientConnectionManager::ConnectToServer (const std::string & uri)
{
    try {
        if (is_running) {
            std::cout << "Already connected or connecting" << std::endl;
            return false;
        }

        // Configure client
        ws_client.clear_access_channels (websocketpp::log::alevel::all);
        ws_client.init_asio ();
        ws_client.set_tls_init_handler ([this] (connection_hdl hdl) {
            return OnTlsInit (hdl);
        });

                                        // Set handlers
        ws_client.set_message_handler ([this] (connection_hdl hdl, client::message_ptr msg) {
            OnMessage (hdl, msg);
        });

        ws_client.set_open_handler ([this] (connection_hdl hdl) {
            OnOpen (hdl);
        });

        ws_client.set_close_handler ([this] (connection_hdl hdl) {
            OnClose (hdl);
        });

        ws_client.set_fail_handler ([this] (connection_hdl hdl) {
            OnFail (hdl);
        });

        // Create connection
        websocketpp::lib::error_code ec;
        connection = ws_client.get_connection (uri, ec);
        if (ec) {
            std::cerr << "Connection creation failed: " << ec.message () << std::endl;
            return false;
        }

        // Connect
        ws_client.connect (connection);

        // Start client run loop in background thread
        is_running = true;
        client_thread = std::thread ([this] () {
            try {
                ws_client.run ();
            } catch (const std::exception & e) {
                std::cerr << "Client run exception: " << e.what () << std::endl;
            }
            is_running = false;
        });

        std::cout << "Connecting to " << uri << "..." << std::endl;
        return true;

    } catch (const std::exception & e) {
        std::cerr << "Connection exception: " << e.what () << std::endl;
        return false;
    }
}

void ClientConnectionManager::Disconnect ()
{
    if (is_running && connection) {
        try {
            ws_client.close (connection->get_handle (),
                             websocketpp::close::status::going_away,
                             "Client disconnecting");
        } catch (const std::exception & e) {
            std::cerr << "Disconnect exception: " << e.what () << std::endl;
        }
    }

    is_running = false;
    is_connected = false;

    if (client_thread.joinable ()) {
        client_thread.join ();
    }

    ClientSessionManager::GetInstance ().Reset ();
}

bool ClientConnectionManager::IsConnected () const
{
    return is_connected.load ();
}

ClientQuizController & ClientConnectionManager::GetQuizController ()
{
    return *quiz_controller;
}
