//
//  DecoderContextPool.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/20.
//

#include "DecoderContextPool.hpp"

DecoderContextPool* DecoderContextPool::instance = nullptr;

DecoderContextPool::DecoderContextPool() {
    
}

DecoderContext* DecoderContextPool::front() {
    decoderMtx.lock();
    DecoderContext* ctx = decoderCtxQueue.front();
    decoderMtx.unlock();
    return ctx;
}

void DecoderContextPool::push(DecoderContext *ctx) {
    decoderMtx.lock();
    decoderCtxQueue.push(ctx);
    decoderMtx.unlock();
}

void DecoderContextPool::pop() {
    decoderMtx.lock();
    decoderCtxQueue.pop();
    decoderMtx.unlock();
}

bool DecoderContextPool::empty() {
    decoderMtx.lock();
    bool isEmpty = decoderCtxQueue.empty();
    decoderMtx.unlock();
    return isEmpty;
}

DecoderContext* DecoderContextPool::getDecoderCtx(const char *inputFileName) {
    DecoderContext *decoderCtx;
    decoderMtx.lock();
    if (!decoderCtxQueue.empty()) {
        decoderCtx = decoderCtxQueue.front();
        decoderCtxQueue.pop();
        decoderMtx.unlock();
    } else {
        decoderMtx.unlock();
        decoderCtx = new DecoderContext(inputFileName);
    }
    return decoderCtx;
}
