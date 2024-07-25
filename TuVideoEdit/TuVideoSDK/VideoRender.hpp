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
#include <vector>
#include <map>

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(__APPLE__)
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <GLKit/GLKMatrix4.h>
#endif

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using namespace std;

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    vector<unsigned int>   Size;      // Size of glyph
    vector<int>   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class VideoRender {
public:
    GLint _backingWidth;
    GLint _backingHeight;
    GLubyte *pixelData = nullptr;
    GLuint frameVAO, frameVBO;
    GLuint stickerVAO, stickerVBO;
    GLuint textVAO, textVBO;
    GLuint frameProgram;
    GLuint stickerProgram;
    GLuint textProgram;
    
    const char *basePath;
    const char *frameVertexPath;
    const char *frameFragPath;
    const char *stickerVertexPath;
    const char *stickerFragPath;
    const char *textVertexPath;
    const char *textFragPath;
    
    std::map<GLchar, Character> Characters;
    
public:
    VideoRender(const char *basePath);
    void setup();
    void setupFrameProgram();
    void setupStickerProgram();
    int setupTextProgram();
#ifdef __APPLE__
    void setupFrameBuffer();
    void setupRenderBuffer();
#endif
    void setupViewport(int width = 0, int height = 0);
    GLuint createProgram(const char *vertexFileName, const char *fragmentFileName);
    void displayFrame(AVFrame *frame);
    AVFrame * convertFrame(AVFrame *frame);
    void draw(AVFrame *frame);
    void drawFrame(AVFrame *frame);
    void drawSticker();
    void drawText();
};


#endif /* VideoRender_hpp */
