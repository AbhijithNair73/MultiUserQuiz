#include "QuestionTimer.h"

QuestionTimer::QuestionTimer (int limit_ms, std::function<void ()> callback)
    : time_limit_ms (limit_ms), timeout_callback (callback)
{ }

QuestionTimer::~QuestionTimer ()
{
    if (running) {
        Stop ();  // Cleanly stop on destruction.
    }
    if (timer_thread.joinable ()) {

        timer_thread.join ();
    }
}

void QuestionTimer::Start ()
{
    start_time = std::chrono::steady_clock::now ();
    running = true;
    answered = false;

    timer_thread = std::thread ([this] () {

        std::unique_lock<std::mutex> lock (mtx);

        if (cv.wait_for (lock, std::chrono::milliseconds (time_limit_ms), [this] () { return answered.load (); })) {

            // User answered before timeout.
            running = false;
            return;
        }

        // Time ran out and question wasn't answered.
        if (!answered) {
            end_time = std::chrono::steady_clock::now ();
            running = false;
            timeout_callback ();
        }
     });
}

// This function is called when the user answers the question.
// It will be called after every question attempt in bullet mode, while
// in timebound mode it will be called once at the end of the test when the user presses 
// "submit"/"EndTest" on the end.
void QuestionTimer::Stop ()
{
    std::lock_guard<std::mutex> lock (mtx);
    if (!answered && running) {
        answered = true;
        end_time = std::chrono::steady_clock::now ();
        cv.notify_one ();  // Wake thread immediately.
    }
}

// This function returns the time left in the test and it should always be positive.
// In bullet mode - if user has answered the question then it will tell the time taken to answer the question.
// In competitive mode - it will tell the time taken by user to complete the entire test.
// At any other point, this function will be called to display
// the time left to attempt the question or to display/sync-up the time left in the test.
long long QuestionTimer::GetElapsedTimeMillis () const
{
    if (answered) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count ();
    }

    auto now = std::chrono::steady_clock::now ();

    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count ();
}