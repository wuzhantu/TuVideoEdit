//
//  TuSemaphore.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#ifndef TuSemaphore_hpp
#define TuSemaphore_hpp

#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class TuSemaphore {
private:
    mutex mtx;
    condition_variable cv;
    int count;
    
public:
    TuSemaphore(int count = 0);
    void notify();
    void wait();
};

#endif /* TuSemaphore_hpp */
