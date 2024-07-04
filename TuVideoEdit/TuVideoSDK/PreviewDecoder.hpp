//
//  PreviewDecoder.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#ifndef PreviewDecoder_hpp
#define PreviewDecoder_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <mutex>

#ifdef __ANDROID__
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#elif defined(__APPLE__)
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#endif

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "TuTimer.hpp"
#include "TuSemaphore.hpp"
#include "DecoderContext.hpp"

using namespace std;

class PreviewDecoder {
    
    
public:
    bool isVideoPause;
    bool isAudioPause;
    TuSemaphore audioSemap;
    mutex videoCondition;
    condition_variable cv;
    TuTimer audioTimer;
    int64_t audioCount;
    TuTimer mDispalyLink;
    int64_t seekAudioPts;
    const char *inputFileName;
    int seekRow = 0;
    bool pause;
    bool isFirstPlay = true;
    
    mutex videoLock;
    
    int64_t currentVideoPts;
    queue<AVFrame *> videoQueue;
    int videoFrameDuration;
    int audioIndex;
    AVCodecContext *audiodec_ctx;
    AVFrame *audiodec_frame;
    AVFrame *aSwrOutFrame;
    mutex audioLock;
    AVFormatContext *audioifmt;
    const AVCodec *audioDec;
    AVPacket *audioPkt;
    enum AVSampleFormat oformat;
    int orate;
    SwrContext *pswr;
    int outChanel;
    std::function<void(AVFrame *)>renderVideoBlock;
    
#ifdef __ANDROID__
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    SLObjectItf outputMixObject = NULL;
    SLObjectItf playerObject = NULL;
    SLPlayItf playerPlay = NULL;
    SLAndroidSimpleBufferQueueItf bufferQueue = NULL;
#elif defined(__APPLE__)
    ALuint sourceId;
#endif
    
public:
    PreviewDecoder(const char *inputFileName, std::function<void(AVFrame *)>renderVideoBlock);
    void setupVideoFFmpeg();
    void setupAudioFFmpeg();
    void videoPreviewDecode(int previewRow);
    void videoPlayDecode();
    void audioPlayDecode();
    void cleanAllVideoBuffer();
    void cleanAllAudioBuffer();
    void setPause(bool pause);
    
    void timerAction();
    void updateFrame();
    
#ifdef __ANDROID__
    void createEngine();
#elif defined(__APPLE__)
    int al_context_create(ALCdevice **pdevice, ALCcontext **pcontext, ALuint *sourceId);
    void playpcm(ALuint sourceId, ALenum format, ALsizei rate, uint8_t *pbuffer, int buffersize);
#endif
    
};

#endif /* PreviewDecoder_hpp */


