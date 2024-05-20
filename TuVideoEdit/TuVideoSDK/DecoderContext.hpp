//
//  DecoderContext.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/15.
//

#ifndef DecoderContext_hpp
#define DecoderContext_hpp

#include <stdio.h>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

using namespace std;

class DecoderContext {
    
private:
    const AVCodec *forwardVideoDec = NULL;
    
public:
    int videoIndex = -1;
    AVCodecContext *forwardVideodecCtx = NULL;
    AVFormatContext *forwardVideoifmt = NULL;
    AVStream *videoStram;
    AVPacket *forwardVideoPkt;
    int videoFrameDuration;
    
    DecoderContext(const char *inputFileName);
};

#endif /* DecoderContext_hpp */
