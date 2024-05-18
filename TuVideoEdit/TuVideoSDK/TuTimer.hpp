//
//  TuTimer.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#ifndef TuTimer_hpp
#define TuTimer_hpp

#include <stdio.h>

#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>

class TuTimer {
public:
    TuTimer();
    ~TuTimer();
    void start(std::function<void()> callback);
    void stop();
    void setInterval(int ms);

private:
    std::atomic<bool> isRunning;
    std::thread timerThread;
    int intervalMs;
};

#endif /* TuTimer_hpp */
