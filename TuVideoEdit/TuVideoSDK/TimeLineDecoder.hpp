//
//  TimeLineDecoder.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/16.
//

#ifndef TimeLineDecoder_hpp
#define TimeLineDecoder_hpp

#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <mutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "DecoderContext.hpp"

using namespace std;

class TimeLineDecoder {

public:
    mutex frameMapMtx;
    mutex decoderMtx;
    unordered_map<int, AVFrame *> frameMap;
    int videoFrameDuration;
    queue<DecoderContext *> decoderCtxQueue;
    const char *inputFileName;
    
    TimeLineDecoder(const char *inputFileName);
    void videoDecode(int startRow, int endRow, std::function<void(int, int)>reloadBlock);
    AVFrame *frameForIndex(int index);
};

#endif /* TimeLineDecoder_hpp */
