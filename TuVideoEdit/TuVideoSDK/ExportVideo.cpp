//
//  ExportVideo.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/16.
//

#include "ExportVideo.hpp"

ExportVideo::ExportVideo(const char *inputFileName, const char *outputPath) {
    this->inputFileName = inputFileName;
    this->outputPath = outputPath;
    setup();
}

void ExportVideo::setup() {
    
    videoIndex = -1;
    videoifmt = NULL;
    videoofmt = NULL;
    
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
    
    avcodec_parameters_to_context(videodec_ctx, videoCodecpar);
    
    ret = avcodec_open2(videodec_ctx, videoDec, NULL);
    if (ret < 0) {
        cout << "videodecoder open failed" << endl;
    }
    
    videoEnc = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!videoEnc) {
        cout << "videoEnc alloc failed" << endl;
    }
    
    videoenc_ctx = avcodec_alloc_context3(videoEnc);
    if (!videoenc_ctx) {
        cout << "videoenc_ctx alloc failed" << endl;
    }
    
    videoenc_ctx->bit_rate = videodec_ctx->bit_rate;
    videoenc_ctx->width = videodec_ctx->width;
    videoenc_ctx->height = videodec_ctx->height;
    videoenc_ctx->time_base = videoStram->time_base;
    videoenc_ctx->framerate = videoStram->r_frame_rate;
    videoenc_ctx->gop_size = videodec_ctx->gop_size;
    videoenc_ctx->max_b_frames = 1;
    videoenc_ctx->pix_fmt = videodec_ctx->pix_fmt;
    
    ret = avcodec_open2(videoenc_ctx, videoEnc, NULL);
    if (ret < 0) {
        cout << "videoEncoder open failed" << endl;
    }
    
    ret = avformat_alloc_output_context2(&videoofmt, NULL, NULL, outputPath);
    if (ret < 0) {
        cout << "videoofmt alloc failed" << endl;
    }
    
    for(int i = 0; i < videoifmt->nb_streams; i++) {
        AVStream *istream = videoifmt->streams[i];
        AVStream *ostream = avformat_new_stream(videoofmt, videoEnc);
        if (istream->index == videoIndex) {
            ret = avcodec_parameters_from_context(ostream->codecpar, videoenc_ctx);
            ostream->time_base = istream->time_base;
            ostream->r_frame_rate = istream->r_frame_rate;
        } else {
            avcodec_parameters_copy(ostream->codecpar, istream->codecpar);
        }
    }
    
    ret = avio_open2(&videoofmt->pb, outputPath, AVIO_FLAG_WRITE, NULL, NULL);
    if (ret < 0) {
        cout << "avio_open2 failed" << endl;
    }
    
    ret = avformat_write_header(videoofmt, NULL);
    if (ret < 0) {
        cout << "avformat_write_header failed" << endl;
    }
    
    videoPkt = av_packet_alloc();
    if (!videoPkt) {
        cout << "create video packet failed" << endl;
    }
    
    videodec_frame = av_frame_alloc();
    if (!videodec_frame) {
        cout << "create videodec_frame failed" << endl;
    }
}

void ExportVideo::videoDecode() {
    
    int ret;
    int64_t audioPts = 0;
    while (1) {
        
        ret = av_read_frame(videoifmt, videoPkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            cout << "av_read_frame failed" << endl;
            goto end;
        }
        
        if (videoPkt->stream_index == videoIndex) {
            ret = avcodec_send_packet(videodec_ctx, videoPkt);
            if (ret < 0) {
                cout << "video decode failed" << endl;
                goto end;
            }
            
            while (ret >= 0) {
                
                ret = avcodec_receive_frame(videodec_ctx, videodec_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    cout << "receive decode video frame failed" << endl;
                    goto end;
                }
                
                AVFrame *filterFrame = videoRender->applyFilterToFrame(videodec_frame);
                ret = avcodec_send_frame(videoenc_ctx, filterFrame);
                if (ret < 0) {
                    cout << "video encode failed" << endl;
                    goto end;
                }
                
                AVPacket *encPkt = av_packet_alloc();
                while (ret >= 0) {
                    ret = avcodec_receive_packet(videoenc_ctx, encPkt);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        cout << "receive encode video packet failed" << endl;
                        goto end;
                    }
                    encPkt->stream_index = videoIndex;
                    av_interleaved_write_frame(videoofmt, encPkt);
                    
                    av_packet_unref(encPkt);
                }
                av_packet_free(&encPkt);
                av_frame_unref(videodec_frame);
                av_frame_unref(filterFrame);
                av_frame_free(&filterFrame);
            }
        } else {
            videoPkt->pts = audioPts;
            videoPkt->dts = audioPts;
            av_interleaved_write_frame(videoofmt, videoPkt);
            audioPts += 1024;
        }
        av_packet_unref(videoPkt);
    }
    
    ret = av_write_trailer(videoofmt);
    if (ret < 0) {
        cout << "av_write_trailer failed" << endl;
    }
    
    cout << "export end" << endl;
    
end:
    cout << "end" << endl;
    av_frame_free(&videodec_frame);
    av_packet_free(&videoPkt);
}
