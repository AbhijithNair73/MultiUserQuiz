#pragma once
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <memory>

enum class TimerType {
    QUESTION_TIMER,    // For individual questions
    QUIZ_TIMER         // For entire quiz duration
};

enum class TimerState {
    NOT_STARTED,
    RUNNING,
    ANSWERED,
    TIMED_OUT,
    FORCE_STOPPED
};

class QuestionTimer {

private:

    long long time_limit_ms;
    std::function<void ()> timeout_callback;
    std::function<void ()> force_stop_callback;  // New callback for force stop

    std::thread timer_thread;
    std::mutex mtx;
    std::condition_variable cv;

    std::atomic<bool> answered{false};
    std::atomic<bool> running{false};
    std::atomic<bool> force_stopped{false};  // New atomic flag

    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    TimerType timer_type;
    TimerState current_state{TimerState::NOT_STARTED};

public:
    QuestionTimer (long long limit_ms,
                   std::function<void ()> callback,
                   TimerType type = TimerType::QUESTION_TIMER,
                   std::function<void ()> force_stop_cb = nullptr);

    ~QuestionTimer ();

    void Start ();
    void Stop ();
    void ForceStop ();  // New method for immediate termination

    long long GetElapsedTimeMillis () const;
    long long GetRemainingTimeMillis () const;

    TimerState GetState () const
    {
        return current_state;
    }
    bool IsRunning () const
    {
        return running.load ();
    }
    bool IsForcesStopped () const
    {
        return force_stopped.load ();
    }
    TimerType GetTimerType () const
    {
        return timer_type;
    }
};