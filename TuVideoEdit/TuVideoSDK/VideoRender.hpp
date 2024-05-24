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
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
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
    vector<int>   Size;      // Size of glyph
    vector<int>   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class VideoRender {
public:
    GLuint myColorRenderBuffer;
    GLuint myColorFrameBuffer;
    GLubyte *pixelData = nullptr;
    GLuint VAO, VBO;
    GLuint stickerVAO, stickerVBO;
    GLuint textVAO, textVBO;
    GLuint myProgram;
    GLuint stickerProgram;
    GLuint textProgram;
    GLint _backingWidth;
    GLint _backingHeight;
    
    const char *basePath;
    const char *displayVertexPath;
    const char *displayFragPath;
    const char *stickerVertexPath;
    const char *stickerFragPath;
    const char *textVertexPath;
    const char *textFragPath;
    
    std::map<GLchar, Character> Characters;
    
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
    
    int setupTextProgram();
    void drawText(string text, float x, float y, float scale, vector<float> color);
};


#endif /* VideoRender_hpp */
