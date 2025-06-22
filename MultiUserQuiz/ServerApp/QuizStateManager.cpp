// QuizStateManager.cpp
#include "QuizStateManager.hpp"

std::unique_ptr<QuizStateManager> QuizStateManager::instance = nullptr;
std::mutex QuizStateManager::instance_mutex;

QuizStateManager & QuizStateManager::GetInstance ()
{
    std::lock_guard<std::mutex> lock (instance_mutex);
    if (!instance) {
        instance = std::unique_ptr<QuizStateManager> (new QuizStateManager ());
    }
    return *instance;
}

void QuizStateManager::StartQuiz (const std::string & quiz_id, long long duration_ms)
{
    std::lock_guard<std::mutex> lock (state_mutex);

    quiz_states[quiz_id] = QuizState::IN_PROGRESS;

    auto timeout_callback = [this, quiz_id] () {
        EndQuiz (quiz_id, QuizState::ENDED_TIMEOUT);
        NotifyClients (quiz_id, "QUIZ_TIMEOUT");
    };

    auto force_stop_callback = [this, quiz_id] () {
        NotifyClients (quiz_id, "QUIZ_FORCE_STOPPED");
    };

    quiz_timers[quiz_id] = std::make_unique<QuestionTimer> (
        duration_ms,
        timeout_callback,
        TimerType::QUIZ_TIMER,
        force_stop_callback
    );

    quiz_timers[quiz_id]->Start ();
}

void QuizStateManager::EndQuiz (const std::string & quiz_id, QuizState end_state)
{
    std::lock_guard<std::mutex> lock (state_mutex);

    auto it = quiz_states.find (quiz_id);
    if (it != quiz_states.end ()) {
        it->second = end_state;
    }

    auto timer_it = quiz_timers.find (quiz_id);
    if (timer_it != quiz_timers.end ()) {
        timer_it->second->ForceStop ();
        quiz_timers.erase (timer_it);
    }
}

void QuizStateManager::ForceEndAllQuizzes ()
{
    std::lock_guard<std::mutex> lock (state_mutex);

    for (auto & [quiz_id, state] : quiz_states) {
        if (state == QuizState::IN_PROGRESS) {
            state = QuizState::ENDED_FORCE_STOPPED;
        }
    }

    for (auto & [quiz_id, timer] : quiz_timers) {
        timer->ForceStop ();
    }

    NotifyAllClients ("ALL_QUIZZES_FORCE_STOPPED");
    quiz_timers.clear ();
}

void QuizStateManager::ForceEndQuiz (const std::string & quiz_id)
{
    EndQuiz (quiz_id, QuizState::ENDED_FORCE_STOPPED);
    NotifyClients (quiz_id, "QUIZ_FORCE_STOPPED");
}

QuizState QuizStateManager::GetQuizState (const std::string & quiz_id) const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    auto it = quiz_states.find (quiz_id);
    return (it != quiz_states.end ()) ? it->second : QuizState::NOT_STARTED;
}

bool QuizStateManager::IsQuizActive (const std::string & quiz_id) const
{
    return GetQuizState (quiz_id) == QuizState::IN_PROGRESS;
}

bool QuizStateManager::CanAccessQuiz (const std::string & quiz_id, const std::string & user_id) const
{
    QuizState state = GetQuizState (quiz_id);

    // Allow access for result checking if quiz has ended
    return state == QuizState::IN_PROGRESS ||
        state == QuizState::ENDED_TIMEOUT ||
        state == QuizState::ENDED_FORCE_STOPPED ||
        state == QuizState::ENDED_COMPLETED;
}

void QuizStateManager::RegisterClient (const std::string & quiz_id,
                                       std::function<void (const std::string &)> callback)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    client_callbacks[quiz_id] = callback;
}

void QuizStateManager::NotifyClients (const std::string & quiz_id, const std::string & message)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    auto it = client_callbacks.find (quiz_id);
    if (it != client_callbacks.end () && it->second) {
        it->second (message);
    }
}

void QuizStateManager::NotifyAllClients (const std::string & message)
{
    std::lock_guard<std::mutex> lock (state_mutex);
    for (auto & [quiz_id, callback] : client_callbacks) {
        if (callback) {
            callback (message);
        }
    }
}

long long QuizStateManager::GetRemainingTime (const std::string & quiz_id) const
{
    std::lock_guard<std::mutex> lock (state_mutex);
    auto it = quiz_timers.find (quiz_id);
    return (it != quiz_timers.end ()) ? it->second->GetRemainingTimeMillis () : 0;
}

/*
// Usage Example
void StrictQuizTimedOut ()
{
// Handle quiz timeout
    QuizStateManager::GetInstance ().ForceEndAllQuizzes ();
}

// Server-side quiz initialization
void InitializeQuiz (const std::string & quiz_id, int quiz_mode, long long time_allowed_in_ms)
{
    if (quiz_mode == STRICT_TIME_BOUND_MODE) {
        QuizStateManager::GetInstance ().StartQuiz (quiz_id, time_allowed_in_ms);
    }
}

// Client-side usage for individual questions
class ClientQuestionTimer {
    private:
    std::unique_ptr<QuestionTimer> question_timer;
    std::function<void ()> answer_callback;

    public:
    ClientQuestionTimer (long long question_time_ms, std::function<void ()> callback)
        : answer_callback (callback)
    {

        auto timeout_cb = [this] () {
            // Handle question timeout
            if (answer_callback) {
                answer_callback ();
            }
        };

        question_timer = std::make_unique<QuestionTimer> (
            question_time_ms,
            timeout_cb,
            TimerType::QUESTION_TIMER
        );
    }

    void StartQuestion ()
    {
        question_timer->Start ();
    }

    void AnswerSelected ()
    {
        question_timer->Stop ();
    }

    long long GetRemainingTime () const
    {
        return question_timer->GetRemainingTimeMillis ();
    }

    bool IsActive () const
    {
        return question_timer->IsRunning ();
    }
};
*/