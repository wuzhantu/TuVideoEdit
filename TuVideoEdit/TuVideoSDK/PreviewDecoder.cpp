//
//  PreviewDecoder.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#include "PreviewDecoder.hpp"
#include "DecoderContextPool.hpp"

PreviewDecoder::PreviewDecoder(const char *inputFileName, std::function<void(AVFrame *)>renderVideoBlock) {
    this->inputFileName = inputFileName;
    this->renderVideoBlock = renderVideoBlock;
    setupVideoFFmpeg();
    setupAudioFFmpeg();
}

void PreviewDecoder::setupVideoFFmpeg() {
    currentVideoPts = 0;
}

void PreviewDecoder::setupAudioFFmpeg() {
    
    seekAudioPts = -1;
    
    audioIndex = -1;
    
    audioifmt = NULL;
    
    audiodec_frame = av_frame_alloc();
    aSwrOutFrame = av_frame_alloc();
    
    int ret = avformat_open_input(&audioifmt, inputFileName, NULL, NULL);
    if (ret < 0) {
        cout << "open inputfile failed" << endl;
    }
    
    ret = avformat_find_stream_info(audioifmt, NULL);
    if (ret < 0) {
        cout << "find input stream failed" << endl;
    }
    
    audioIndex = av_find_best_stream(audioifmt, AVMEDIA_TYPE_AUDIO, -1, -1, &audioDec, 0);
    if (audioIndex < 0) {
        cout << "not find input audio stream" << endl;
    }
    
    audiodec_ctx = avcodec_alloc_context3(audioDec);
    if (!audiodec_ctx) {
        cout << "audiodec_ctx alloc failed" << endl;
    }
    
    avcodec_parameters_to_context(audiodec_ctx, audioifmt->streams[audioIndex]->codecpar);
    
    ret = avcodec_open2(audiodec_ctx, audioDec, NULL);
    if (ret < 0) {
        cout << "audiodecoder open failed" << endl;
    }
    
    audioPkt = av_packet_alloc();
    if (!audioPkt) {
        cout << "create audio packet failed" << endl;
    }
    
    oformat = AV_SAMPLE_FMT_S16;
    int outLayout = AV_CH_LAYOUT_STEREO;
    orate = 44100;
    pswr = NULL;
    outChanel = av_get_channel_layout_nb_channels(outLayout);
    pswr = swr_alloc_set_opts(pswr, outLayout, oformat, orate, audiodec_ctx->channel_layout, audiodec_ctx->sample_fmt, audiodec_ctx->sample_rate, 0, 0);
    ret = swr_init(pswr);
    
#ifdef __ANDROID__
    createEngine();
#elif defined(__APPLE__)
    sourceId = 0;
    ALCdevice *pdevice = NULL;
    ALCcontext *pcontext = NULL;
    ret = al_context_create(&pdevice, &pcontext, &sourceId);
#endif
}

void PreviewDecoder::videoPreviewDecode(int previewRow) {
    
    DecoderContext *decoderCtx = DecoderContextPool::shareInstance()->getDecoderCtx(inputFileName);
    
    int videoIndex = decoderCtx->videoIndex;
    AVCodecContext *previewVideodecCtx = decoderCtx->forwardVideodecCtx;
    AVFormatContext *previewVideoifmt = decoderCtx->forwardVideoifmt;
    AVPacket *previewVideoPkt = decoderCtx->forwardVideoPkt;
    int videoFrameDuration = decoderCtx->videoFrameDuration;
    
    av_packet_unref(previewVideoPkt);
    avcodec_flush_buffers(previewVideodecCtx);
    
    int ret;
    
    if (seekRow != previewRow) {
        goto end;
    }
    
    ret = av_seek_frame(previewVideoifmt, -1, previewRow * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        cout << "seek frame failed" << endl;
        goto end;
    }
    
    while (seekRow == previewRow) {
        
        ret = av_read_frame(previewVideoifmt, previewVideoPkt);
        if (ret < 0) {
            break;
        }
        
        if (previewVideoPkt->stream_index == videoIndex && seekRow == previewRow) {
            ret = avcodec_send_packet(previewVideodecCtx, previewVideoPkt);
            if (ret < 0) {
                cout << "video decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0 && seekRow == previewRow) {
                
                AVFrame *videodec_frame = av_frame_alloc();
                ret = avcodec_receive_frame(previewVideodecCtx, videodec_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_frame_free(&videodec_frame);
                    break;
                } else if (ret < 0) {
                    cout << "receive decode video frame failed" << endl;
                    av_frame_free(&videodec_frame);
                    goto end;
                }
                
                if (videodec_frame->pts == previewRow * videoFrameDuration) {
                    renderVideoBlock(videodec_frame);
                    goto end;
                } else {
                    av_frame_free(&videodec_frame);
                }
            }
        }
        
        av_packet_unref(previewVideoPkt);
    }
    
end:
    DecoderContextPool::shareInstance()->push(decoderCtx);
}

void PreviewDecoder::videoPlayDecode() {
    
    DecoderContext *decoderCtx = DecoderContextPool::shareInstance()->getDecoderCtx(inputFileName);
    
    int videoIndex = decoderCtx->videoIndex;
    AVCodecContext *playVideodecCtx = decoderCtx->forwardVideodecCtx;
    AVFormatContext *playVideoifmt = decoderCtx->forwardVideoifmt;
    AVPacket *playVideoPkt = decoderCtx->forwardVideoPkt;
    int videoFrameDuration = decoderCtx->videoFrameDuration;
    
    av_packet_unref(playVideoPkt);
    avcodec_flush_buffers(playVideodecCtx);
    
    int ret = av_seek_frame(playVideoifmt, -1, seekRow * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        cout << "seek frame failed" << endl;
        goto end;
    }
    
    while (!isVideoPause) {
        
        ret = av_read_frame(playVideoifmt, playVideoPkt);
        if (ret < 0) {
            break;
        }
        
        if (playVideoPkt->stream_index == videoIndex) {
            ret = avcodec_send_packet(playVideodecCtx, playVideoPkt);
            if (ret < 0) {
                cout << "video decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0) {
                if (isVideoPause) {
                    goto end;
                }
                
                AVFrame *videodec_frame = av_frame_alloc();
                ret = avcodec_receive_frame(playVideodecCtx, videodec_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_frame_free(&videodec_frame);
                    break;
                } else if (ret < 0) {
                    cout << "receive decode video frame failed" << endl;
                    av_frame_free(&videodec_frame);
                    goto end;
                }
                
                if (videodec_frame->pts > seekRow * videoFrameDuration) {
                    
                    unique_lock<mutex> lock(videoCondition);
                    if (videoQueue.size() >= 30) {
                        cv.wait(lock);
                    }
                    videoQueue.push(videodec_frame);
                    cv.notify_one();
                    
                } else {
                    av_frame_free(&videodec_frame);
                }
            }
        }
        
        av_packet_unref(playVideoPkt);
    }
    
end:
    DecoderContextPool::shareInstance()->push(decoderCtx);
}

void PreviewDecoder::audioPlayDecode() {
    
    audioLock.lock();
    
    av_packet_unref(audioPkt);
    avcodec_flush_buffers(audiodec_ctx);
    av_frame_unref(audiodec_frame);
    av_frame_unref(aSwrOutFrame);
    
    int ret = av_seek_frame(audioifmt, -1, seekRow * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        cout << "seek frame failed" << endl;
    }
    
    while (!isAudioPause) {
        ret = av_read_frame(audioifmt, audioPkt);
        
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            cout << "read audio frame failed" << endl;
            break;
        }
        
        if (audioPkt->stream_index == audioIndex) {

            if (isAudioPause) {
                goto end;
            }
            
            ret = avcodec_send_packet(audiodec_ctx, audioPkt);
            if (ret < 0) {
                cout << "audio decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0) {
                if (isAudioPause) {
                    goto end;
                }
                
                ret = avcodec_receive_frame(audiodec_ctx, audiodec_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    cout << "receive audio frame failed" << endl;
                    goto end;
                }
                
                if (isAudioPause) {
                    goto end;
                }
                
                if (seekAudioPts == -1) {
                    seekAudioPts = audioPkt->pts;
                }
                
                int64_t dstnbSamples = av_rescale_rnd(swr_get_delay(pswr, audiodec_ctx->sample_rate) + audiodec_frame->nb_samples, orate, audiodec_frame->sample_rate, AV_ROUND_UP);
                ret = av_samples_alloc(aSwrOutFrame->data, aSwrOutFrame->linesize, outChanel, (int)dstnbSamples, oformat, 0);
                ret = swr_convert(pswr, aSwrOutFrame->data, (int)dstnbSamples, (const uint8_t **)audiodec_frame->data, audiodec_frame->nb_samples);
                int writingSize = av_samples_get_buffer_size(aSwrOutFrame->linesize, outChanel, ret, oformat, 1);
                
#ifdef __ANDROID__
                audioSemap.wait();
                (*bufferQueue)->Enqueue(bufferQueue, aSwrOutFrame->data[0], writingSize);
#elif defined(__APPLE__)
                playpcm(sourceId, AL_FORMAT_STEREO16, audiodec_ctx->sample_rate, aSwrOutFrame->data[0], writingSize);
#endif
                
                av_frame_unref(audiodec_frame);
                av_frame_unref(aSwrOutFrame);
            }
        }
        
        av_packet_unref(audioPkt);
    }
    
end:
    audioLock.unlock();
}

#ifdef __ANDROID__

#elif defined(__APPLE__)
int PreviewDecoder::al_context_create(ALCdevice **pdevice, ALCcontext **pcontext, ALuint *sourceId) {
    //播放源的位置
    ALfloat position[] = { 0.0f,0.0f,0.0f };
    //播放的速度
    ALfloat velocity[] = { 0.0f,0.0f,0.0f };
    
    *pdevice = alcOpenDevice(NULL);
    if (!(*pdevice)) {
        return -1;
    }
    
    *pcontext = alcCreateContext(*pdevice, NULL);
    if (!(*pcontext)) {
        return -1;
    }
    
    alcMakeContextCurrent(*pcontext);
    
    alGenSources(1, sourceId);
    //音高倍数
    alSourcef(*sourceId, AL_PITCH, 1.0f);
    //声音的增益
    alSourcef(*sourceId, AL_GAIN, 1.0f);
    //设置位置
    alSourcefv(*sourceId, AL_POSITION, position);
    //设置声音移动速度
    alSourcefv(*sourceId, AL_VELOCITY, velocity);
    //设置是否循环播放
    alSourcei(*sourceId, AL_LOOPING, AL_FALSE);
    
    return 0;
}
#endif

void PreviewDecoder::cleanAllVideoBuffer() {
    videoCondition.lock();
    while (!videoQueue.empty()) {
        AVFrame *display_frame = videoQueue.front();
        videoQueue.pop();
        av_frame_unref(display_frame);
        av_frame_free(&display_frame);
    }
    videoCondition.unlock();
}

void PreviewDecoder::cleanAllAudioBuffer() {
#ifdef __ANDROID__

#elif defined(__APPLE__)
    ALint queued = 0;
    alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queued);
    if (queued > 0) {
        ALuint *queuedBufPtr = (ALuint *)malloc(sizeof(ALuint) * queued);
        alSourceUnqueueBuffers(sourceId, queued, queuedBufPtr);
        alDeleteBuffers(queued, queuedBufPtr);
    }
#endif
}

#ifdef __ANDROID__
static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    PreviewDecoder *previewDecoder = (PreviewDecoder *)context;
    previewDecoder->audioSemap.notify();
}

void PreviewDecoder::createEngine() {
    SLresult result;

    // Create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        cout << "slCreateEngine failed" << endl;
        return;
    }

    // Realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Engine realization failed" << endl;
        return;
    }

    // Get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Engine interface creation failed" << endl;
        return;
    }

    // Create output mix
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Output mix creation failed" << endl;
        return;
    }

    // Realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Output mix realization failed" << endl;
        return;
    }

    // Configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // Configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // Create audio player
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Audio player creation failed" << endl;
        return;
    }

    // Realize the player
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Player realization failed" << endl;
        return;
    }

    // Get the play interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Play interface creation failed" << endl;
        return;
    }

    // Get the buffer queue interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &bufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Buffer queue interface creation failed" << endl;
        return;
    }

    // Register callback on the buffer queue
    result = (*bufferQueue)->RegisterCallback(bufferQueue, playerCallback, this);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Buffer queue callback registration failed" << endl;
        return;
    }

    // Set the player's state to playing
    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS) {
        cout << "Failed to set play state" << endl;
        return;
    }

    // Start playback by enqueueing an initial buffer
    playerCallback(bufferQueue, this);

    cout << "Engine and output mix created successfully" << endl;
}

#elif defined(__APPLE__)
void PreviewDecoder::playpcm(ALuint sourceId, ALenum format, ALsizei rate, uint8_t *pbuffer, int buffersize) {
    
    if (isAudioPause) {
        return;
    }
    
    ALuint bufferID = 0;
    alGenBuffers(1, &bufferID);
    if (bufferID == 0) {
        audioSemap.wait();
        if (isAudioPause) {
            return;
        }
        alGenBuffers(1, &bufferID);
    }
    alBufferData(bufferID, AL_FORMAT_STEREO16, pbuffer, buffersize, rate);
    alSourceQueueBuffers(sourceId, 1, &bufferID);// 入队bufferID
    ALint stateVaue;
    alGetSourcei(sourceId, AL_SOURCE_STATE, &stateVaue);//获取状态
    if (stateVaue != AL_PLAYING)
    {
        alSourcePlay(sourceId);
        audioCount = 0;
        audioTimer.setInterval(50);
        audioTimer.start([this]() {
            this->timerAction();
        });
    }
}
#endif

void PreviewDecoder::timerAction() {
    if (isAudioPause) {
        return;
    }

#ifdef __ANDROID__

#elif defined(__APPLE__)
    audioCount++;
    ALint processed = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    if (processed > 0) {
        ALuint *processedBufPtr = (ALuint *)malloc(sizeof(ALuint) * processed);
        alSourceUnqueueBuffers(sourceId, processed, processedBufPtr);
        alDeleteBuffers(processed, processedBufPtr);
        audioSemap.notify();
    }
#endif
    
}

void PreviewDecoder::updateFrame() {
    
    if (isVideoPause) {
        return;
    }
    
    unique_lock<mutex> lock(videoCondition);
    if (videoQueue.size() == 0) {
        cv.wait(lock);
    }
    AVFrame *display_frame = videoQueue.front();
    renderVideoBlock(display_frame);
    videoQueue.pop();
    cv.notify_one();
}

//
//- (void)updateFrameSync {
//
//    if (isVideoPause) {
//        return;
//    }
//
//    [videoCondition lock];
//    if (videoQueue.empty()) {
//        [videoCondition unlock];
//        return;
//    }
//
//    AVFrame *display_frame = videoQueue.front();
//    [videoCondition unlock];
//    int result = [self compareAVTime:display_frame];
//    if (result == 1) {
//        return;
//    }
//
//    [videoCondition lock];
//    if (videoQueue.size() == 0) {
//        [videoCondition wait];
//    }
//
//    if (videoQueue.size() > 0) {
//        if (result == 0) {
//            renderVideoBlock(display_frame);
//            videoQueue.pop();
//        } else if (result < 0) {
//            while (!videoQueue.empty()) {
//                display_frame = videoQueue.front();
//                result = [self compareAVTime:display_frame];
//                if (result >= 0) {
//                    break;
//                }
//                videoQueue.pop();
//                av_frame_unref(display_frame);
//                av_frame_free(&display_frame);
//            }; // 跳帧
//        } else {
//            // 等待
//        }
//    }
//
//    [videoCondition signal];
//    [videoCondition unlock];
//}

//- (int)compareAVTime:(AVFrame *)display_frame {
//    currentVideoPts = display_frame->pts;
//    double threshold = 0.05; //0.05
//    double videotime = 1.0 / videoFrameDuration * currentVideoPts;
//    double audiotime = audioCount * 0.05 + 1.0 / 44100.0 * seekAudioPts;
//    double difftime = videotime - audiotime;
//
//    if (fabs(difftime) <= threshold) {
//        return 0; // 播放
//    } else if (difftime < 0) {
//        return -1; // 跳帧
//    } else {
//        return 1; // 等待
//    }
//}

void PreviewDecoder::setPause(bool pause) {
    this->pause = pause;
    
    if (pause) {
        isVideoPause = true;
        mDispalyLink.stop();
        cv.notify_all();
        
        isAudioPause = true;
        audioSemap.notify();
        
        seekAudioPts = -1;
        
#ifdef __ANDROID__

#elif defined(__APPLE__)
        alSourcePause(sourceId);
        alSourceRewind(sourceId);
        //    alSourceStop(sourceId);
#endif
        
        
        audioTimer.stop();
    } else {
        isVideoPause = false;
        isAudioPause = false;

        if (isFirstPlay) {
            DecoderContext *decoderCtx = DecoderContextPool::shareInstance()->getDecoderCtx(inputFileName);
            int fps = decoderCtx->videoStram->r_frame_rate.num / decoderCtx->videoStram->r_frame_rate.den;
            mDispalyLink.setInterval(1000 / fps);
            isFirstPlay = false;
        }
        mDispalyLink.start([this]() {
            this->updateFrame();
        });
        
        cleanAllVideoBuffer();
        cleanAllAudioBuffer();
    }
}
