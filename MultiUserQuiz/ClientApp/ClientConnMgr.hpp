#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <memory>

// Type alias for TLS client
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

class ClientConnMgr {

    public: 
        ClientConnMgr () = default;
        ~ClientConnMgr () = default;

        static void ConnectToServer ();

};