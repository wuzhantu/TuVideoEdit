//
//  ExportVideo.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/16.
//

#ifndef ExportVideo_hpp
#define ExportVideo_hpp

#include <stdio.h>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include "VideoRender.hpp"

using namespace std;

class ExportVideo {
public:
    const char *inputFileName;
    const char *outputPath;
    int videoIndex;
    AVFormatContext *videoifmt;
    AVFormatContext *videoofmt;
    const AVCodec *videoDec;
    const AVCodec *videoEnc;
    AVCodecContext *videodec_ctx;
    AVCodecContext *videoenc_ctx;
    AVFrame *videodec_frame;
    AVPacket *videoPkt;
    VideoRender *videoRender;
    
public:
    ExportVideo(const char *inputFileName, const char *outputPath);
    void setup();
    void videoDecode();
};

#endif /* ExportVideo_hpp */
