
// SessionManager.hpp
#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <websocketpp/connection.hpp>
#include <User.h>

using connection_hdl = websocketpp::connection_hdl;

class SessionManager {
    private:
    static std::unique_ptr<SessionManager> instance;
    static std::mutex instance_mutex;

    std::map<connection_hdl, std::string, std::owner_less<connection_hdl>> hdl_to_username;
    std::unordered_map<std::string, std::shared_ptr<User>> username_to_user;
    mutable std::shared_mutex session_mutex;

    SessionManager () = default;

    public:
    static SessionManager & GetInstance ();

    // Session management
    bool AddSession (connection_hdl hdl, const std::string & username);
    bool RemoveSession (connection_hdl hdl);
    std::string GetUsername (connection_hdl hdl) const;
//    connection_hdl GetConnectionHandle (const std::string & username) const;

    // User management
    std::shared_ptr<User> GetUser (const std::string & username) const;
    std::shared_ptr<User> GetUserByHandle (connection_hdl hdl) const;
    bool CreateUser (const std::string & username);

    // Connection state
    bool IsUserLoggedIn (const std::string & username) const;
//    bool IsHandleValid (connection_hdl hdl) const;

    // Notification support
    void NotifyUser (const std::string & username, const std::string & message);
    void NotifyAllUsers (const std::string & message);

    // Session validation
    bool ValidateSession (connection_hdl hdl, std::string & error_msg) const;
};
