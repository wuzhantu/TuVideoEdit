//
//  DecoderContextPool.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/20.
//

#ifndef DecoderContextPool_hpp
#define DecoderContextPool_hpp

#include <stdio.h>
#include <queue>
#include <mutex>
#include "DecoderContext.hpp"

using namespace std;

class DecoderContextPool {
private:
    static DecoderContextPool* instance;
    queue<DecoderContext *> decoderCtxQueue;
    mutex decoderMtx;
    
public:
    static DecoderContextPool* shareInstance() {
        if (instance == nullptr) {
            instance = new DecoderContextPool();
        }
        return instance;
    }
    
    DecoderContextPool();
    DecoderContext* front();
    void push(DecoderContext *ctx);
    void pop();
    bool empty();
    DecoderContext *getDecoderCtx(const char *inputFileName);
};



#endif /* DecoderContextPool_hpp */
