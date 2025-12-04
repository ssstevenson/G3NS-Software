#ifndef __GEN3_M2M_TIMER__
#define __GEN3_M2M_TIMER__

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

class M2MTimer {
public:
    M2MTimer() : timerThread(), mtx(), cv(),timerStopFlag(false) {}

    // Start the timer
    void startTimer(int seconds) 
    {
        stopTimer(); // Ensure any previous timer is stopped before starting a new one

        timerThread = std::thread([this, seconds]() 
        {
            std::unique_lock<std::mutex>  lock(mtx);
            if (!cv.wait_for(lock, std::chrono::seconds(seconds), [this]() { return timerStopFlag; })) 
            {
                timerExpired();  // Timer expired, call event handler
            } 
            else 
            {
                std::cout << "Timer was stopped before expiration!" << std::endl;
            }
        });
    }

    // Stop the timer
    void stopTimer() 
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            timerStopFlag = true;
        }
        cv.notify_one(); // Wake up the timer thread
        if (timerThread.joinable()) 
        {
            timerThread.join(); // Ensure thread is cleaned up
        }
    }

    // Function called when timer expires
    virtual void timerExpired() {
        std::cout << "Timer expired! Calling event handler..." << std::endl;
    }

    virtual ~M2MTimer() {
        stopTimer(); // Clean up when object is destroyed
    }

private:
    std::thread timerThread;
    std::mutex mtx;
    std::condition_variable cv;
    bool timerStopFlag;
};

#endif
