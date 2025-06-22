#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include "QuestionTimer.h"

enum class QuizState {
    NOT_STARTED,
    IN_PROGRESS,
    ENDED_TIMEOUT,
    ENDED_FORCE_STOPPED,
    ENDED_COMPLETED
};

class QuizStateManager {
    private:
    static std::unique_ptr<QuizStateManager> instance;
    static std::mutex instance_mutex;

    std::unordered_map<std::string, QuizState> quiz_states;
    std::unordered_map<std::string, std::unique_ptr<QuestionTimer>> quiz_timers;
    std::unordered_map<std::string, std::function<void (const std::string &)>> client_callbacks;

    mutable std::mutex state_mutex;

    QuizStateManager () = default;

    public:
    static QuizStateManager & GetInstance ();

    // Quiz management
    void StartQuiz (const std::string & quiz_id, long long duration_ms);
    void EndQuiz (const std::string & quiz_id, QuizState end_state);
    void ForceEndAllQuizzes ();
    void ForceEndQuiz (const std::string & quiz_id);

    // State queries
    QuizState GetQuizState (const std::string & quiz_id) const;
    bool IsQuizActive (const std::string & quiz_id) const;
    bool CanAccessQuiz (const std::string & quiz_id, const std::string & user_id) const;

    // Client management
    void RegisterClient (const std::string & quiz_id,
                         std::function<void (const std::string &)> callback);
    void NotifyClients (const std::string & quiz_id, const std::string & message);
    void NotifyAllClients (const std::string & message);

    // Timer access
    long long GetRemainingTime (const std::string & quiz_id) const;
};