//
//  PreviewDecoder.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/17.
//

#include "PreviewDecoder.hpp"

PreviewDecoder::PreviewDecoder(const char *inputFileName, std::function<void(AVFrame *)>renderVideoBlock) {
    this->inputFileName = inputFileName;
    this->renderVideoBlock = renderVideoBlock;
    setupVideoFFmpeg();
    setupAudioFFmpeg();
}

void PreviewDecoder::setupVideoFFmpeg() {
    
    videoIndex = -1;
    currentVideoPts = 0;
    
    videoifmt = NULL;
    
    int ret = avformat_open_input(&videoifmt, inputFileName, NULL, NULL);
    if (ret < 0) {
        cout << "open inputfile failed" << endl;
    }
    
    ret = avformat_find_stream_info(videoifmt, NULL);
    if (ret < 0) {
        cout << "find input stream failed" << endl;
    }
    
    videoIndex = av_find_best_stream(videoifmt, AVMEDIA_TYPE_VIDEO, -1, -1, &videoDec, 0);
    if (videoIndex < 0) {
        cout << "not find input video stream" << endl;
    }
    
    videodec_ctx = avcodec_alloc_context3(videoDec);
    if (!videodec_ctx) {
        cout << "videodec_ctx alloc failed" << endl;
    }
    
    AVStream *videoStram = videoifmt->streams[videoIndex];
    AVCodecParameters *videoCodecpar = videoStram->codecpar;
    videoFrameDuration = videoStram->time_base.den / videoStram->time_base.num;
    
    avcodec_parameters_to_context(videodec_ctx, videoCodecpar);
    
    ret = avcodec_open2(videodec_ctx, videoDec, NULL);
    if (ret < 0) {
        cout << "videodecoder open failed" << endl;
    }
    
    videoPkt = av_packet_alloc();
    if (!videoPkt) {
        cout << "create video packet failed" << endl;
    }
    
    int fps = videoStram->r_frame_rate.num / videoStram->r_frame_rate.den;
    mDispalyLink.setInterval(1000 / fps);
    mDispalyLink.start([this]() {
        this->updateFrame();
    });
}

void PreviewDecoder::setupAudioFFmpeg() {
    seekAudioPts = -1;
    
    sourceId = 0;
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
    
    oformat = AV_SAMPLE_FMT_S16;
    int outLayout = AV_CH_LAYOUT_STEREO;
    orate = 44100;
    pswr = NULL;
    ALCdevice *pdevice = NULL;
    ALCcontext *pcontext = NULL;
    outChanel = av_get_channel_layout_nb_channels(outLayout);
    ret = al_context_create(&pdevice, &pcontext, &sourceId);
    pswr = swr_alloc_set_opts(pswr, outLayout, oformat, orate, audiodec_ctx->channel_layout, audiodec_ctx->sample_fmt, audiodec_ctx->sample_rate, 0, 0);
    ret = swr_init(pswr);
    
    audioPkt = av_packet_alloc();
    if (!audioPkt) {
        cout << "create audio packet failed" << endl;
    }
}

void PreviewDecoder::videoPreviewDecode(int previewRow) {
    
    videoLock.lock();
    
    av_packet_unref(videoPkt);
    avcodec_flush_buffers(videodec_ctx);
    
    int ret;
    
    if (seekRow != previewRow) {
        goto end;
    }
    
    ret = av_seek_frame(videoifmt, -1, previewRow * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        cout << "seek frame failed" << endl;
        goto end;
    }
    
    while (seekRow == previewRow) {
        
        ret = av_read_frame(videoifmt, videoPkt);
        if (ret < 0) {
            break;
        }
        
        if (videoPkt->stream_index == videoIndex) {
            ret = avcodec_send_packet(videodec_ctx, videoPkt);
            if (ret < 0) {
                cout << "video decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0 && seekRow == previewRow) {
                
                AVFrame *videodec_frame = av_frame_alloc();
                ret = avcodec_receive_frame(videodec_ctx, videodec_frame);
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
        
        av_packet_unref(videoPkt);
    }
    
    cout << "preview videoDecode end" << endl;
    
end:
    videoLock.unlock();
}

void PreviewDecoder::videoPlayDecode() {
    
    videoLock.lock();
    
    av_packet_unref(videoPkt);
    avcodec_flush_buffers(videodec_ctx);
    
    int ret = av_seek_frame(videoifmt, -1, seekRow * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        cout << "seek frame failed" << endl;
        goto end;
    }
    
    while (!isVideoPause) {
        
        ret = av_read_frame(videoifmt, videoPkt);
        if (ret < 0) {
            break;
        }
        
        if (videoPkt->stream_index == videoIndex) {
            ret = avcodec_send_packet(videodec_ctx, videoPkt);
            if (ret < 0) {
                cout << "video decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0) {
                if (isVideoPause) {
                    goto end;
                }
                
                AVFrame *videodec_frame = av_frame_alloc();
                ret = avcodec_receive_frame(videodec_ctx, videodec_frame);
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
        
        av_packet_unref(videoPkt);
    }
    
    cout << "display videoDecode end" << endl;
    
end:
    videoLock.unlock();
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
                
                playpcm(sourceId, AL_FORMAT_STEREO16, audiodec_ctx->sample_rate, aSwrOutFrame->data[0], writingSize);
                
                av_frame_unref(audiodec_frame);
                av_frame_unref(aSwrOutFrame);
            }
        }
        
        av_packet_unref(audioPkt);
    }
    
    cout << "display audioDecode end" << endl;
    
end:
    audioLock.unlock();
}

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
    ALint queued = 0;
    alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queued);
    if (queued > 0) {
        ALuint *queuedBufPtr = (ALuint *)malloc(sizeof(ALuint) * queued);
        alSourceUnqueueBuffers(sourceId, queued, queuedBufPtr);
        alDeleteBuffers(queued, queuedBufPtr);
    }
}

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

void PreviewDecoder::timerAction() {
    if (isAudioPause) {
        return;
    }

    audioCount++;
    ALint processed = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    if (processed > 0) {
        ALuint *processedBufPtr = (ALuint *)malloc(sizeof(ALuint) * processed);
        alSourceUnqueueBuffers(sourceId, processed, processedBufPtr);
        alDeleteBuffers(processed, processedBufPtr);
        audioSemap.notify();
    }
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
//        audioSemap = dispatch_semaphore_create(0);
        
        seekAudioPts = -1;
        alSourcePause(sourceId);
        alSourceRewind(sourceId);
    //    alSourceStop(sourceId);
        
        audioTimer.stop();
    } else {
        isVideoPause = false;
        isAudioPause = false;

        mDispalyLink.start([this]() {
            this->updateFrame();
        });
        
        cleanAllVideoBuffer();
        cleanAllAudioBuffer();
    }
}
