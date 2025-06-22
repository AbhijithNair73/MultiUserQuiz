// ConnectionManager.cpp
#include "ConnectionManager.hpp"
#include <iostream>

ConnectionManager::ConnectionManager ()
    : quiz_controller (std::make_unique<QuizController> ())
{ }

ConnectionManager::~ConnectionManager ()
{
    StopServer ();
}

std::shared_ptr<asio::ssl::context> ConnectionManager::OnTlsInit (connection_hdl hdl)
{
    auto ctx = std::make_shared<asio::ssl::context> (asio::ssl::context::tlsv12);
    try {
        ctx->set_options (
            asio::ssl::context::default_workarounds |
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::single_dh_use);

        ctx->use_certificate_chain_file ("server.crt");
        ctx->use_private_key_file ("server.key", asio::ssl::context::pem);

        std::cout << "TLS init succeeded." << std::endl;
    } catch (std::exception & e) {
        std::cout << "TLS init failed: " << e.what () << std::endl;
    }
    return ctx;
}

void ConnectionManager::OnMessage (server * s, connection_hdl hdl, server::message_ptr msg)
{
    try {
#ifdef DEBUG
        server::connection_ptr con = s->get_con_from_hdl (hdl);
        std::string client_info = con->get_remote_endpoint ();
        std::cout << "[MESSAGE] From " << client_info << ": " << msg->get_payload () << std::endl;
#endif

        json request = json::parse (msg->get_payload ());
        json response = quiz_controller->ProcessRequest (hdl, request);

        s->send (hdl, response.dump (), websocketpp::frame::opcode::text);

    } catch (const std::exception & e) {
        std::cerr << "Message handling exception: " << e.what () << std::endl;

        json error_response = {{"type", "ERROR"}, {"message", "Internal server error"}};
        s->send (hdl, error_response.dump (), websocketpp::frame::opcode::text);
    }
}

void ConnectionManager::OnOpen (server * s, connection_hdl hdl)
{
    auto con = s->get_con_from_hdl (hdl);
    std::string remote = con->get_remote_endpoint ();
    std::cout << "[CONNECTED] " << remote << std::endl;

    quiz_controller->OnConnect (hdl);
}

void ConnectionManager::OnClose (server * s, connection_hdl hdl)
{
    auto con = s->get_con_from_hdl (hdl);
    std::string remote = con->get_remote_endpoint ();
    std::cout << "[DISCONNECTED] " << remote << std::endl;

    quiz_controller->OnDisconnect (hdl);
}

void ConnectionManager::StartServer (int port)
{
    try {
        ws_server.set_access_channels (websocketpp::log::alevel::none);
        ws_server.init_asio ();
        ws_server.set_tls_init_handler (std::bind (&ConnectionManager::OnTlsInit, this, std::placeholders::_1));
        ws_server.set_message_handler (std::bind (&ConnectionManager::OnMessage, this, &ws_server, std::placeholders::_1, std::placeholders::_2));
        ws_server.set_open_handler (std::bind (&ConnectionManager::OnOpen, this, &ws_server, std::placeholders::_1));
        ws_server.set_close_handler (std::bind (&ConnectionManager::OnClose, this, &ws_server, std::placeholders::_1));

        ws_server.listen (port);
        ws_server.start_accept ();

        // Create thread pool
        const int num_threads = std::thread::hardware_concurrency ();

        for (int i = 0; i < num_threads; ++i) {
            thread_pool.emplace_back ([this] () {
                ws_server.run ();
                                      });
        }

        std::cout << "Server started on port " << port << " with " << num_threads << " threads" << std::endl;

        // Join threads
        for (auto & t : thread_pool) {
            if (t.joinable ()) {
                t.join ();
            }
        }

    } catch (const std::exception & e) {
        std::cerr << "Server error: " << e.what () << std::endl;
    }
}

void ConnectionManager::StopServer ()
{
    // Force end all active quizzes before shutdown
    QuizStateManager::GetInstance ().ForceEndAllQuizzes ();

    // Stop WebSocket server
    ws_server.stop ();

    // Join all threads
    for (auto & t : thread_pool) {
        if (t.joinable ()) {
            t.join ();
        }
    }

    thread_pool.clear ();
}