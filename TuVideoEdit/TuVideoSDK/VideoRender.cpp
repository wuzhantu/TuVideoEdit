//
//  VideoRender.cpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/16.
//

#include "VideoRender.hpp"
#include "VideoRenderConfig.hpp"

VideoRender::VideoRender(const char *displayVertexPath, const char *displayFragPath, const char *stickerVertexPath, const char *stickerFragPath) {
    this->displayVertexPath = displayVertexPath;
    this->displayFragPath = displayFragPath;
    this->stickerVertexPath = stickerVertexPath;
    this->stickerFragPath = stickerFragPath;
    setup();
}

void VideoRender::setup() {
    setupProgram();
    setupFrameBuffer();
    setupRenderBuffer();
}

void VideoRender::setupFrameBuffer() {
    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    myColorFrameBuffer = frameBuffer;
}

void VideoRender::setupRenderBuffer() {
    GLuint renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);
    myColorRenderBuffer = renderBuffer;
}

void VideoRender::setupViewport() {
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    glViewport(0, 0, _backingWidth, _backingHeight);
}

void VideoRender::setupProgram() {
    myProgram = createProgram(displayVertexPath, displayFragPath);
    
    GLfloat vertices[] = {
        -1.0f, 1.0f, 0.0f, 0.0f, 0.5f,       0.0f, 0.0f,       0.0f, 0.25f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.5f,       0.5f, 0.0f,       0.5f, 0.25f,
        -1.0f,  -1.0f, 0.0f, 0.0f, 1.0f,       0.0f, 0.25f,      0.0f, 0.5f,
        1.0f,  -1.0f, 0.0f,  1.0f, 1.0f,       0.5f, 0.25f,      0.5f, 0.5f,
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (void *)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (void *)(sizeof(GLfloat) * 5));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (void *)(sizeof(GLfloat) * 7));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    stickerProgram = createProgram(stickerVertexPath, stickerFragPath);
    
    GLfloat stickerVertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  -0.5f, 0.0f, 0.0f, 1.0f,
        0.5f,  -0.5f, 0.0f,  1.0f, 1.0f,
    };
    
    GLuint stickerVBO;
    glGenVertexArrays(1, &stickerVAO);
    glGenBuffers(1, &stickerVBO);
    glBindVertexArray(stickerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, stickerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(stickerVertices), stickerVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void *)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint VideoRender::createProgram(const char *vertexFileName, const char *fragmentFileName) {
    
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexFileName);
        fShaderFile.open(fragmentFileName);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    
    
    GLuint program = glCreateProgram();

    const GLchar *vertexContentStr = vShaderCode;
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexContentStr, NULL);
    glCompileShader(vertexShader);
    glAttachShader(program, vertexShader);
    
    const GLchar *fragContentStr = fShaderCode;
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragContentStr, NULL);
    glCompileShader(fragShader);
    glAttachShader(program, fragShader);
    
    glLinkProgram(program);
    
    GLint linkSuccess;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) { //连接错误
        GLchar messages[256];
        glGetProgramInfoLog(program, sizeof(messages), 0, &messages[0]);
        cout << messages << endl;
        return program;
    }
    else {
        cout << "link ok" << endl;
    }
    
    return program;
}

void VideoRender::displayFrame(AVFrame *frame) {
    draw(frame);
    av_frame_unref(frame);
    av_frame_free(&frame);
}

AVFrame * VideoRender::applyFilterToFrame(AVFrame *frame) {
    draw(frame);
    
    if (!pixelData) {
        pixelData = (GLubyte *)malloc(_backingWidth * _backingHeight * 4); // RGBA每个像素4个字节
    }
    memset(pixelData, 0, _backingWidth * _backingHeight * 4);
    glReadPixels(0, 0, _backingWidth, _backingHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

    // 创建临时缓冲区，并将像素数据写入其中
    GLubyte *tempBuffer = (GLubyte *)malloc(_backingWidth * _backingHeight * 4);
    memcpy(tempBuffer, pixelData, _backingWidth * _backingHeight * 4);

    // 垂直翻转像素数据
    for (int y = 0; y < _backingHeight / 2; ++y) {
        for (int x = 0; x < _backingWidth * 4; ++x) {
            GLubyte temp = tempBuffer[y * _backingWidth * 4 + x];
            tempBuffer[y * _backingWidth * 4 + x] = tempBuffer[(_backingHeight - y - 1) * _backingWidth * 4 + x];
            tempBuffer[(_backingHeight - y - 1) * _backingWidth * 4 + x] = temp;
        }
    }
    
    // 设置转换参数
    struct SwsContext *sws_ctx = sws_getContext(_backingWidth, _backingHeight, AV_PIX_FMT_RGBA,
                                                _backingWidth, _backingHeight, AV_PIX_FMT_YUV420P,
                                                 SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        // 创建转换上下文失败，处理错误
    }

    int dstStride[AV_NUM_DATA_POINTERS] = {0};
    dstStride[0] = _backingWidth * 4;

    AVFrame *retFrame = av_frame_alloc();
    retFrame->format = AV_PIX_FMT_YUV420P;
    retFrame->width = frame->width;
    retFrame->height = frame->height;
    int ret = av_frame_get_buffer(retFrame, 0);
    av_frame_copy_props(retFrame, frame);

    // 执行转换
    sws_scale(sws_ctx, &tempBuffer, dstStride, 0, _backingHeight,
              retFrame->data, retFrame->linesize);
    
    free(tempBuffer);
    sws_freeContext(sws_ctx);
    
    return retFrame;
}

void VideoRender::draw(AVFrame *frame) {
    if (!frame) return;
    
    int frameWidth = frame->width;
    int frameHeight = frame->height;
    if (frameWidth == 0 || frameHeight == 0) {
        return;
    }
    
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    GLuint yuvTexture;
    glGenTextures(1, &yuvTexture);
    glBindTexture(GL_TEXTURE_2D, yuvTexture);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[0], frameHeight * 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, frameHeight, frame->linesize[0], frameHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, frameHeight / 2, frame->linesize[1], frameHeight / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[2], frameHeight / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[2]);
    
    glUseProgram(myProgram);
    
    glBindTexture(GL_TEXTURE_2D, yuvTexture);
    
    glUniform1i(glGetUniformLocation(myProgram, "applyInversionFilter"), VideoRenderConfig::shareInstance()->applyInversionFilter);
    glUniform1i(glGetUniformLocation(myProgram, "applyGrayscaleFilter"), VideoRenderConfig::shareInstance()->applyGrayscaleFilter);
    glUniform1i(glGetUniformLocation(myProgram, "applyEffect1"), VideoRenderConfig::shareInstance()->applyEffect1);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    GLfloat ratio = 1.0 * frame->width / frame->linesize[0];
    if (VideoRenderConfig::shareInstance()->applyEffect2) {
        GLfloat vertices1[] = {
            -1.0f, 0.0f, 0.0f, 0.0f,       1.0f, 0.0f,       0.5f,  0.0f,       0.25f,
             0.0f, 0.0f, 0.0f, 1.0f*ratio, 1.0f, 0.5f*ratio, 0.5f,  0.5f*ratio, 0.25f,
            -1.0f, 1.0f, 0.0f, 0.0f,       0.5f, 0.0f,       0.25f, 0.0f,       0.0f,
             0.0f, 1.0f, 0.0f, 1.0f*ratio, 0.5f, 0.5f*ratio, 0.25f, 0.5f*ratio, 0.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        GLfloat vertices2[] = {
            0.0f, 0.0f, 0.0f, 0.0f,       1.0f, 0.0f,       0.5f,  0.0f,       0.25f,
            1.0f, 0.0f, 0.0f, 1.0f*ratio, 1.0f, 0.5f*ratio, 0.5f,  0.5f*ratio, 0.25f,
            0.0f, 1.0f, 0.0f, 0.0f,       0.5f, 0.0f,       0.25f, 0.0f,       0.0f,
            1.0f, 1.0f, 0.0f, 1.0f*ratio, 0.5f, 0.5f*ratio, 0.25f, 0.5f*ratio, 0.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        GLfloat vertices3[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,       1.0f, 0.0f,       0.5f,  0.0f,       0.25f,
             0.0f, -1.0f, 0.0f, 1.0f*ratio, 1.0f, 0.5f*ratio, 0.5f,  0.5f*ratio, 0.25f,
            -1.0f,  0.0f, 0.0f, 0.0f,       0.5f, 0.0f,       0.25f, 0.0f,       0.0f,
             0.0f,  0.0f, 0.0f, 1.0f*ratio, 0.5f, 0.5f*ratio, 0.25f, 0.5f*ratio, 0.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3), vertices3, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        GLfloat vertices4[] = {
            0.0f, -1.0f, 0.0f, 0.0f,       1.0f, 0.0f,       0.5f,  0.0f,       0.25f,
            1.0f, -1.0f, 0.0f, 1.0f*ratio, 1.0f, 0.5f*ratio, 0.5f,  0.5f*ratio, 0.25f,
            0.0f,  0.0f, 0.0f, 0.0f,       0.5f, 0.0f,       0.25f, 0.0f,       0.0f,
            1.0f,  0.0f, 0.0f, 1.0f*ratio, 0.5f, 0.5f*ratio, 0.25f, 0.5f*ratio, 0.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices4), vertices4, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } else {
        GLfloat vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,       1.0f, 0.0f,       0.5f,  0.0f,       0.25f,
             1.0f, -1.0f, 0.0f, 1.0f*ratio, 1.0f, 0.5f*ratio, 0.5f,  0.5f*ratio, 0.25f,
            -1.0f,  1.0f, 0.0f, 0.0f,       0.5f, 0.0f,       0.25f, 0.0f,       0.0f,
             1.0f,  1.0f, 0.0f, 1.0f*ratio, 0.5f, 0.5f*ratio, 0.25f, 0.5f*ratio, 0.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    if (VideoRenderConfig::shareInstance()->applySticker1) {
        static int frameCount1 = 1;
        if (frameCount1 > 81) {
            frameCount1 = 1;
            VideoRenderConfig::shareInstance()->applySticker1 = false;
        } else {
            GLuint stickerTexture;
            glGenTextures(1, &stickerTexture);
            glBindTexture(GL_TEXTURE_2D, stickerTexture);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//            GLubyte *spriteData = [self getBitmapImage:[NSString stringWithFormat:@"tiger_%04d.png", frameCount1++]];
            GLubyte *spriteData;
            float fw = 500, fh = 498;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fw, fh, 0, GL_RGBA, GL_UNSIGNED_BYTE, spriteData);
            
            glUseProgram(stickerProgram);
            glBindVertexArray(stickerVAO);
            glBindTexture(GL_TEXTURE_2D, stickerTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    } else if (VideoRenderConfig::shareInstance()->applySticker2) {
        static int frameCount2 = 1;
        if (frameCount2 > 100) {
            frameCount2 = 1;
            VideoRenderConfig::shareInstance()->applySticker2 = false;
        } else {
            GLuint stickerTexture;
            glGenTextures(1, &stickerTexture);
            glBindTexture(GL_TEXTURE_2D, stickerTexture);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//            GLubyte *spriteData = [self getBitmapImage:[NSString stringWithFormat:@"airplane_%04d.png", frameCount2++]];
            GLubyte *spriteData;
            float fw = 500, fh = 498;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fw, fh, 0, GL_RGBA, GL_UNSIGNED_BYTE, spriteData);
            
            glUseProgram(stickerProgram);
            glBindVertexArray(stickerVAO);
            glBindTexture(GL_TEXTURE_2D, stickerTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
    
    glDeleteTextures(1, &yuvTexture);
}
