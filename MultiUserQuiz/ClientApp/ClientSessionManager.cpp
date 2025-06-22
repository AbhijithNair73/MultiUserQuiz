// ClientSessionManager.cpp
#include "ClientSessionManager.hpp"

std::unique_ptr<ClientSessionManager> ClientSessionManager::instance = nullptr;
std::mutex ClientSessionManager::instance_mutex;

ClientSessionManager & ClientSessionManager::GetInstance ()
{
    std::lock_guard<std::mutex> lock (instance_mutex);
    if (!instance) {
        instance = std::unique_ptr<ClientSessionManager> (new ClientSessionManager ());
    }
    return *instance;
}

void ClientSessionManager::SetState (ClientState state)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    current_state = state;
}

ClientState ClientSessionManager::GetState () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return current_state;
}

void ClientSessionManager::SetUsername (const std::string & username)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    current_username = username;
}

std::string ClientSessionManager::GetUsername () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return current_username;
}

void ClientSessionManager::SetReconnection (bool is_recon)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    is_reconnection = is_recon;
}

bool ClientSessionManager::IsReconnection () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return is_reconnection;
}

void ClientSessionManager::UpdateQuizConfig (const json & config)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    total_questions = config.value ("total_questions", 0);
    quiz_mode = config.value ("quiz_mode", 0);
    is_multioption_allowed = config.value ("is_multioption_allowed", false);
    is_kbc_mode = config.value ("is_kbc_mode", false);
    total_time_limit = config.value ("total_time", 0LL);
    end_time = config.value ("end_time", 0LL);
    elapsed_time = config.value ("updated_elapsed_time", 0LL);
}

void ClientSessionManager::UpdateQuizProgress (const json & progress)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    if (progress.contains ("updated_elapsed_time")) {
        elapsed_time = progress["updated_elapsed_time"];
    }
    if (progress.contains ("score")) {
        current_score = progress["score"];
    }
}

void ClientSessionManager::SetCurrentQuestion (unsigned int qid)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    current_question_id = qid;
}

unsigned int ClientSessionManager::GetCurrentQuestion () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return current_question_id;
}

void ClientSessionManager::UpdateUnattemptedQuestions (const std::vector<unsigned int> & questions)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    unattempted_questions = questions;
}

std::vector<unsigned int> ClientSessionManager::GetUnattemptedQuestions () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return unattempted_questions;
}

// Getters implementation
unsigned int ClientSessionManager::GetTotalQuestions () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return total_questions;
}

long long ClientSessionManager::GetTotalTimeLimit () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return total_time_limit;
}

long long ClientSessionManager::GetElapsedTime () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return elapsed_time;
}

long long ClientSessionManager::GetEndTime () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return end_time;
}

double ClientSessionManager::GetCurrentScore () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return current_score;
}

int ClientSessionManager::GetQuizMode () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return quiz_mode;
}

bool ClientSessionManager::IsMultiOptionAllowed () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return is_multioption_allowed;
}

bool ClientSessionManager::IsKBCMode () const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    return is_kbc_mode;
}

void ClientSessionManager::Reset ()
{
    std::lock_guard<std::mutex> lock (state_mutex);
    current_username.clear ();
    current_state = ClientState::DISCONNECTED;
    is_reconnection = false;
    total_questions = 0;
    current_question_id = 0;
    unattempted_questions.clear ();
    total_time_limit = 0;
    elapsed_time = 0;
    end_time = 0;
    current_score = 0.0;
}