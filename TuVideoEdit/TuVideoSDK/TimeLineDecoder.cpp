//
//  TimeLineDecoder.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/16.
//

#include "TimeLineDecoder.hpp"
#include "DecoderContextPool.hpp"

TimeLineDecoder::TimeLineDecoder(const char *inputFileName) {
    this->inputFileName = inputFileName;
}

void TimeLineDecoder::videoDecode(int startRow, int endRow, std::function<void(int, int)>reloadBlock) {
    
    if (startRow > endRow) {
        return;
    }
    
    DecoderContext *decoderCtx = DecoderContextPool::shareInstance()->getDecoderCtx(inputFileName);
    
    int videoIndex = decoderCtx->videoIndex;
    AVCodecContext *forwardVideodecCtx = decoderCtx->forwardVideodecCtx;
    AVFormatContext *forwardVideoifmt = decoderCtx->forwardVideoifmt;
    AVPacket *forwardVideoPkt = decoderCtx->forwardVideoPkt;
    int videoFrameDuration = decoderCtx->videoFrameDuration;
    
    avcodec_flush_buffers(forwardVideodecCtx);
    av_packet_unref(forwardVideoPkt);
    
    int currentRow = startRow;
    int count = 0;
//    int lastStartRow = startRow;
    
    int ret = av_seek_frame(forwardVideoifmt, -1, startRow * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        cout << "seek frame failed" << endl;
        goto end;
    }
    
    while (1) {
        
        ret = av_read_frame(forwardVideoifmt, forwardVideoPkt);
        if (ret < 0) {
            break;
        }
        
        if (forwardVideoPkt->stream_index == videoIndex) {
            
            ret = avcodec_send_packet(forwardVideodecCtx, forwardVideoPkt);
            if (ret < 0) {
                cout << "video decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0) {
                
                AVFrame *videodec_frame = av_frame_alloc();
                ret = avcodec_receive_frame(forwardVideodecCtx, videodec_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_frame_free(&videodec_frame);
                    break;
                } else if (ret < 0) {
                    cout << "receive decode video frame failed" << endl;
                    av_frame_free(&videodec_frame);
                    goto end;
                }
                

                bool isFinish = videodec_frame->pts == endRow * videoFrameDuration;
                if (videodec_frame->pts >= startRow * videoFrameDuration && videodec_frame->pts % videoFrameDuration == 0) {
                    frameMapMtx.lock();
                    frameMap[currentRow++] = videodec_frame;
                    frameMapMtx.unlock();
                    count++;
//                    if (isFinish || count % 8 == 0) {
//                        if (reloadBlock) {
//                            reloadBlock(lastStartRow, MIN(lastStartRow + 7, endRow));
//                        }
//                        lastStartRow += 8;
//                    }
                } else {
                    av_frame_free(&videodec_frame);
                }
                
                if (isFinish) {
                    if (reloadBlock) {
                        reloadBlock(startRow, endRow);
                    }
                    goto end;
                }
            }
        }
        
        av_packet_unref(forwardVideoPkt);
    }
    
    cout << "timeline forward videoDecode end" << endl;
    
end:
    DecoderContextPool::shareInstance()->push(decoderCtx);
}

AVFrame * TimeLineDecoder::frameForIndex(int index) {
    frameMapMtx.lock();
    if (frameMap.empty() || frameMap.find(index) == frameMap.end()) {
        frameMapMtx.unlock();
        return NULL;
    }
    AVFrame *frame = frameMap[index];
    frameMapMtx.unlock();
    return frame;
}
