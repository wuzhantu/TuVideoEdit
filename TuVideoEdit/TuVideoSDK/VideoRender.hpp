//
//  VideoRender.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/16.
//

#ifndef VideoRender_hpp
#define VideoRender_hpp

#include <stdio.h>

#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(__APPLE__)
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#endif

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using namespace std;

class VideoRender {
public:
    GLuint myColorRenderBuffer;
    GLuint myColorFrameBuffer;
    GLubyte *pixelData = nullptr;
    GLuint VAO, stickerVAO;
    GLuint VBO, stickerVBO;
    GLuint myProgram;
    GLuint stickerProgram;
    GLint _backingWidth;
    GLint _backingHeight;
    
    const char *basePath;
    const char *displayVertexPath;
    const char *displayFragPath;
    const char *stickerVertexPath;
    const char *stickerFragPath;
    
public:
    VideoRender(const char *basePath);
    void setup();
    void setupFrameBuffer();
    void setupRenderBuffer();
    void setupViewport();
    void setupProgram();
    GLuint createProgram(const char *vertexFileName, const char *fragmentFileName);
    void displayFrame(AVFrame *frame);
    AVFrame * applyFilterToFrame(AVFrame *frame);
    void draw(AVFrame *frame);
};


#endif /* VideoRender_hpp */
