// SessionManager.cpp
#include "SessionManager.hpp"

std::unique_ptr<SessionManager> SessionManager::instance = nullptr;
std::mutex SessionManager::instance_mutex;

SessionManager & SessionManager::GetInstance ()
{
    std::lock_guard<std::mutex> lock (instance_mutex);
    if (!instance) {
        instance = std::unique_ptr<SessionManager> (new SessionManager ());
    }
    return *instance;
}

bool SessionManager::AddSession (connection_hdl hdl, const std::string & username)
{
    std::unique_lock lock (session_mutex);

    // Check if user already connected from another handle
    if (IsUserLoggedIn (username)) {
        return false;
    }

    hdl_to_username[hdl] = username;
    return true;
}

bool SessionManager::RemoveSession (connection_hdl hdl)
{
    std::unique_lock lock (session_mutex);

    auto it = hdl_to_username.find (hdl);
    if (it != hdl_to_username.end ()) {

        hdl_to_username.erase (it);
        return true;
    }
    return false;
}

std::string SessionManager::GetUsername (connection_hdl hdl) const
{
    std::shared_lock lock (session_mutex);
    auto it = hdl_to_username.find (hdl);
    return (it != hdl_to_username.end ()) ? it->second : "";
}

std::shared_ptr<User> SessionManager::GetUser (const std::string & username) const
{
    std::shared_lock lock (session_mutex);
    auto it = username_to_user.find (username);
    return (it != username_to_user.end ()) ? it->second : nullptr;
}

std::shared_ptr<User> SessionManager::GetUserByHandle (connection_hdl hdl) const
{
    std::string username = GetUsername (hdl);
    return username.empty () ? nullptr : GetUser (username);
}

bool SessionManager::CreateUser (const std::string & username)
{
    std::unique_lock lock (session_mutex);
    if (username_to_user.find (username) != username_to_user.end ()) {
        return false; // User already exists
    }
    username_to_user[username] = std::make_shared<User> (username);
    return true;
}

void SessionManager::NotifyUser (const std::string & username, const std::string & message)
{
}

void SessionManager::NotifyAllUsers (const std::string & message)
{
}

bool SessionManager::ValidateSession (connection_hdl hdl, std::string & error_msg) const
{
    std::shared_lock lock (session_mutex);
    auto it = hdl_to_username.find (hdl);
    if (it == hdl_to_username.end ()) {
        error_msg = "Please login first";
        return false;
    }
    return true;
}

bool SessionManager::IsUserLoggedIn (const std::string & username) const
{
    std::shared_lock lock (session_mutex);

    for (const auto & [hdl, existing_user] : hdl_to_username) {
        if (existing_user == username) {
            return true;
        }
    }
    return false;
}