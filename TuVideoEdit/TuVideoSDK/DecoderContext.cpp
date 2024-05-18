//
//  DecoderContext.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/15.
//

#include "DecoderContext.hpp"

DecoderContext::DecoderContext(const char *inputFileName) {
    
    int ret = avformat_open_input(&forwardVideoifmt, inputFileName, NULL, NULL);
    if (ret < 0) {
        cout << "open inputfile failed" << endl;
    }
    
    ret = avformat_find_stream_info(forwardVideoifmt, NULL);
    if (ret < 0) {
        cout << "find input stream failed" << endl;
    }
    
    videoIndex = av_find_best_stream(forwardVideoifmt, AVMEDIA_TYPE_VIDEO, -1, -1, &forwardVideoDec, 0);
    if (videoIndex < 0) {
        cout << "not find input video stream" << endl;
    }
    
    forwardVideodecCtx = avcodec_alloc_context3(forwardVideoDec);
    forwardVideodecCtx->width = 100;
    forwardVideodecCtx->height = 720.0 / 1172.0 * 100;
    if (!forwardVideodecCtx) {
        cout << "videodec_ctx alloc failed" << endl;
    }
    
    AVStream *videoStram = forwardVideoifmt->streams[videoIndex];
    AVCodecParameters *videoCodecpar = videoStram->codecpar;
    videoFrameDuration = videoStram->time_base.den / videoStram->time_base.num;
    
    avcodec_parameters_to_context(forwardVideodecCtx, videoCodecpar);
    
    ret = avcodec_open2(forwardVideodecCtx, forwardVideoDec, NULL);
    if (ret < 0) {
        cout << "videodecoder open failed" << endl;
    }
    
    forwardVideoPkt = av_packet_alloc();
    if (!forwardVideoPkt) {
        cout << "create video packet failed" << endl;
    }
}
