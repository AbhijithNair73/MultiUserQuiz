
#include "ClientConnMgr.hpp"

// TLS init handler returning a context pointer
std::shared_ptr<websocketpp::lib::asio::ssl::context>
on_tls_init (websocketpp::connection_hdl)
{
    auto ctx = std::make_shared<websocketpp::lib::asio::ssl::context> (
        websocketpp::lib::asio::ssl::context::tlsv12_client);

    try {
        // Optionally load system CA certificates
        ctx->set_verify_mode (websocketpp::lib::asio::ssl::verify_none);
        std::cout << "TLS init succeeded." << std::endl;
        // Uncomment if you have a specific CA file
        // ctx->load_verify_file("ca.pem");
    } catch (std::exception & e) {
        std::cerr << "TLS init failed: " << e.what () << std::endl;
    }

    return ctx;
}

void ClientConnMgr::ConnectToServer ()
{
    client c;

    try {
        // Disable logs (optional)
        c.clear_access_channels (websocketpp::log::alevel::all);
        c.init_asio ();

        c.set_tls_init_handler (&on_tls_init);

        websocketpp::lib::error_code ec;
        client::connection_ptr conn = c.get_connection ("wss://127.0.0.1:9002", ec);
        if (ec) {
            std::cerr << "Connection failed: " << ec.message () << std::endl;
            return;
        }

        conn->set_message_handler (
            [&c] (websocketpp::connection_hdl hdl, client::message_ptr msg) {
                std::cout << "Received: " << msg->get_payload () << std::endl;
            });

        c.connect (conn);

        // Start client run loop in a background thread
        std::thread t ([&c] () { c.run (); });

        // Read input from stdin and send messages
        while (true) {
            std::string input;
            std::getline (std::cin, input);
            if (input == "exit") break;
            conn->send (input);
        }

        // Gracefully close the connection
        c.close (conn->get_handle (), websocketpp::close::status::going_away, "");
        t.join ();
    } catch (std::exception & e) {
        std::cerr << "Exception: " << e.what () << std::endl;
    }

}