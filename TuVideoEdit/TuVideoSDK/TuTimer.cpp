//
//  TuTimer.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#include "TuTimer.hpp"

TuTimer::TuTimer() : isRunning(false), intervalMs(40) {} // 默认每16ms调用一次, 大约60FPS

TuTimer::~TuTimer() {
    stop();
}

void TuTimer::start(std::function<void()> callback) {
    if (isRunning.load()) {
        stop();
    }
    isRunning = true;
    timerThread = std::thread([=]() {
        while (isRunning.load()) {
            auto start = std::chrono::steady_clock::now();
            callback();
            auto end = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs) - (end - start));
        }
    });
}

void TuTimer::stop() {
    isRunning = false;
    if (timerThread.joinable()) {
        timerThread.join();
    }
}

void TuTimer::setInterval(int ms) {
    intervalMs = ms;
}
