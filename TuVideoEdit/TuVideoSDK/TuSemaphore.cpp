//
//  TuSemaphore.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#include "TuSemaphore.hpp"

TuSemaphore::TuSemaphore(int count) : count(count) {}

void TuSemaphore::notify() {
    unique_lock<std::mutex> lock(mtx);
    ++count;
    cv.notify_one();
}

void TuSemaphore::wait() {
    unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return count > 0; });
    --count;
}
