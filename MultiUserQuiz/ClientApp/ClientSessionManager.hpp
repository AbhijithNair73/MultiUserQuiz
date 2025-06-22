#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class ClientState {
    DISCONNECTED,
    CONNECTED,
    LOGGED_IN,
    QUIZ_ACTIVE,
    QUIZ_ENDED
};

class ClientSessionManager {

private:
    static std::unique_ptr<ClientSessionManager> instance;
    static std::mutex instance_mutex;

    std::string current_username;
    ClientState current_state;
    bool is_reconnection;
    mutable std::mutex state_mutex;

    // Quiz related state
    unsigned int total_questions;
    unsigned int current_question_id;
    std::vector<unsigned int> unattempted_questions;
    long long total_time_limit;
    long long elapsed_time;
    long long end_time;
    double current_score;
    int quiz_mode;
    bool is_multioption_allowed;
    bool is_kbc_mode;

    ClientSessionManager () : current_state (ClientState::DISCONNECTED), is_reconnection (false)
    { }

public:

    static ClientSessionManager & GetInstance ();

    // State management
    void SetState (ClientState state);
    ClientState GetState () const;

    // Session management
    void SetUsername (const std::string & username);
    std::string GetUsername () const;
    void SetReconnection (bool is_recon);
    bool IsReconnection () const;

    // Quiz state management
    void UpdateQuizConfig (const json & config);
    void UpdateQuizProgress (const json & progress);
    void SetCurrentQuestion (unsigned int qid);
    unsigned int GetCurrentQuestion () const;
    void UpdateUnattemptedQuestions (const std::vector<unsigned int> & questions);
    std::vector<unsigned int> GetUnattemptedQuestions () const;

    // Getters for quiz info
    unsigned int GetTotalQuestions () const;
    long long GetTotalTimeLimit () const;
    long long GetElapsedTime () const;
    long long GetEndTime () const;
    double GetCurrentScore () const;
    int GetQuizMode () const;
    bool IsMultiOptionAllowed () const;
    bool IsKBCMode () const;

    // Reset
    void Reset ();
};