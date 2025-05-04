#include <iostream>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

/*
* This class will be called for each question and hence each client will have its own instance of this class.
* So on a machine/exe there wont be more than one instance of this class at a time.
* So this class works for both the modes - if it is bullet mode then on timeout it will call the callback function.
* Similarly for time bound mode - it will call the callback function on timeout i.e the indicator for end of test.
* 
* On answer attempt the timer will stop 
* 
*/
class QuestionTimer {

    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    int time_limit_ms;                                      // holds the timelimit - in bullet mode it will be 15 sec, for timebound mode it will be total time of test.

    std::atomic<bool> answered{false};
    std::atomic<bool> running{false};
    std::function<void ()> timeout_callback;

    std::thread timer_thread;
    std::mutex mtx;
    std::condition_variable cv;

 public:
 
                QuestionTimer       (int limit_ms, std::function<void ()> callback);
                ~QuestionTimer      ();

     void       Start               ();
     void       Stop                ();

     long long GetElapsedTimeMillis () const;


};
