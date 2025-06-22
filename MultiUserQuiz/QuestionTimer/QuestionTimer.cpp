#include "QuestionTimer.h"

QuestionTimer::QuestionTimer (long long limit_ms,
                              std::function<void ()> callback,
                              TimerType type,
                              std::function<void ()> force_stop_cb)
    : time_limit_ms (limit_ms),
    timeout_callback (callback),
    force_stop_callback (force_stop_cb),
    timer_type (type)
{ }

QuestionTimer::~QuestionTimer ()
{
    if (running.load ()) {
        ForceStop ();
    }
    if (timer_thread.joinable ()) {
        timer_thread.join ();
    }
}

void QuestionTimer::Start ()
{
    Stop (); // Ensure previous thread is cleaned up

    {
        std::lock_guard<std::mutex> lock (mtx);
        answered.store (false);
        force_stopped.store (false);
        running.store (true);
        current_state = TimerState::RUNNING;
    }

    start_time = std::chrono::steady_clock::now ();

    timer_thread = std::thread ([this] () {
        std::unique_lock<std::mutex> lock (mtx);

        // Wait for either answer, timeout, or force stop
        bool completed = cv.wait_for (lock, std::chrono::milliseconds (time_limit_ms),
                                      [this] () {
                                          return answered.load () || force_stopped.load ();
                                      });

        if (force_stopped.load ()) {
            current_state = TimerState::FORCE_STOPPED;
            running.store (false);
            if (force_stop_callback) {
                lock.unlock ();
                force_stop_callback ();
            }
            return;
        }

        if (completed && answered.load ()) {
            current_state = TimerState::ANSWERED;
            running.store (false);
            return;
        }

        // Timeout occurred
        if (!answered.load () && !force_stopped.load ()) {
            end_time = std::chrono::steady_clock::now ();
            current_state = TimerState::TIMED_OUT;
            running.store (false);
            if (timeout_callback) {
                lock.unlock ();
                timeout_callback ();
            }
        }
                                });
}

void QuestionTimer::Stop ()
{
    {
        std::lock_guard<std::mutex> lock (mtx);

        if (!running.load (std::memory_order_acquire)) {
            if (timer_thread.joinable ()) {
                timer_thread.join ();
            }
            return;
        }

        answered.store (true, std::memory_order_release);
        end_time = std::chrono::steady_clock::now ();
        current_state = TimerState::ANSWERED;
        cv.notify_one ();
    }

    if (timer_thread.joinable ()) {
        timer_thread.join ();
    }
    running.store (false, std::memory_order_release);
}

void QuestionTimer::ForceStop ()
{
    {
        std::lock_guard<std::mutex> lock (mtx);

        if (!running.load (std::memory_order_acquire)) {
            return;
        }

        force_stopped.store (true, std::memory_order_release);
        end_time = std::chrono::steady_clock::now ();
        current_state = TimerState::FORCE_STOPPED;
        cv.notify_one ();
    }

    if (timer_thread.joinable ()) {
        timer_thread.join ();
    }
    running.store (false, std::memory_order_release);
}

long long QuestionTimer::GetElapsedTimeMillis () const
{
    if (answered.load () || force_stopped.load ()) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count ();
    }

    auto now = std::chrono::steady_clock::now ();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now - start_time).count ();
}

long long QuestionTimer::GetRemainingTimeMillis () const
{
    if (!running.load ()) {
        return 0;
    }

    long long elapsed = GetElapsedTimeMillis ();
    return std::max (0LL, time_limit_ms - elapsed);
}