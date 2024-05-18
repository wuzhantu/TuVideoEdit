//
//  DisplayView.m
//  CaptureOpenGLES
//
//  Created by zhantu wu on 2024/1/24.
//

#import "DisplayView.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <AVFoundation/AVUtilities.h>

@interface DisplayView ()
@property (nonatomic, strong) EAGLContext *myContext;
@property (nonatomic, strong) CAEAGLLayer *myEagLayer;
@property (nonatomic, assign) CGFloat videoScale;
@end

@implementation DisplayView

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame videoScale:(CGFloat)scale
{
    self = [super initWithFrame:frame];
    if (self) {
        self.videoScale = scale;
        [self setupContext];
        [self setupLayer];
        
        NSString *displayVertexPath = [[NSBundle mainBundle] pathForResource:@"display" ofType:@"vsh"];
        NSString *displayFragPath = [[NSBundle mainBundle] pathForResource:@"display" ofType:@"fsh"];
        NSString *stickerVertexPath = [[NSBundle mainBundle] pathForResource:@"sticker" ofType:@"vsh"];
        NSString *stickerFragPath = [[NSBundle mainBundle] pathForResource:@"sticker" ofType:@"fsh"];
        const char *displayVertexPathCstr = [displayVertexPath cStringUsingEncoding:NSUTF8StringEncoding];
        const char *displayFragPathCstr = [displayFragPath cStringUsingEncoding:NSUTF8StringEncoding];
        const char *stickerVertexPathCstr = [stickerVertexPath cStringUsingEncoding:NSUTF8StringEncoding];
        const char *stickerFragPathCstr = [stickerFragPath cStringUsingEncoding:NSUTF8StringEncoding];
        
        videoRender = new VideoRender(displayVertexPathCstr, displayFragPathCstr, stickerVertexPathCstr, stickerFragPathCstr);
        [self.myContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:self.myEagLayer];
        videoRender->setupViewport();
    }
    return self;
}

- (void)setupContext {
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES3;
    self.myContext = [[EAGLContext alloc] initWithAPI:api];
    [EAGLContext setCurrentContext:self.myContext];
}

- (void)setupLayer {
    self.myEagLayer = (CAEAGLLayer *)self.layer;
    [self setContentScaleFactor:self.videoScale];
    self.myEagLayer.opaque = YES;
}

- (void)displayFrame:(AVFrame *)frame {
    videoRender->displayFrame(frame);
    [self.myContext presentRenderbuffer:GL_RENDERBUFFER];
}

- (AVFrame *)applyFilterToFrame:(AVFrame *)frame {
    return videoRender->applyFilterToFrame(frame);
}

- (GLubyte *)getBitmapImage:(NSString *)imageName {
    CGImageRef spriteImage = [UIImage imageNamed:imageName].CGImage;
    if (!spriteImage) {
        NSLog(@"Failed to load image %@", @"for_test");
        exit(1);
    }

    // 2 读取图片的大小
    size_t width = CGImageGetWidth(spriteImage);
    size_t height = CGImageGetHeight(spriteImage);

    GLubyte * spriteData = (GLubyte *) calloc(width * height * 4, sizeof(GLubyte)); //rgba共4个byte

    CGContextRef spriteContext = CGBitmapContextCreate(spriteData, width, height, 8, width*4,
                                                       CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);

    // 3在CGContextRef上绘图
    CGContextDrawImage(spriteContext, CGRectMake(0, 0, width, height), spriteImage);

    CGContextRelease(spriteContext);
    
    return spriteData;
}

@end
