#pragma once
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

typedef websocketpp::server<websocketpp::config::asio_tls> server;
using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using json = nlohmann::json;
using namespace std;

class ConnectionMgr {

    public:
        static void StartServer ();
};