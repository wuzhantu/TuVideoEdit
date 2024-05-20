//
//  DisplayView.h
//  CaptureOpenGLES
//
//  Created by zhantu wu on 2024/1/24.
//

#import <UIKit/UIKit.h>

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "VideoRender.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface DisplayView : UIView
{
    @public VideoRender *videoRender;
}

@property (nonatomic, assign) BOOL applyInversionFilter;
@property (nonatomic, assign) BOOL applyGrayscaleFilter;

@property (nonatomic, assign) BOOL applySticker1;
@property (nonatomic, assign) BOOL applySticker2;

@property (nonatomic, assign) BOOL applyEffect1;
@property (nonatomic, assign) BOOL applyEffect2;

- (instancetype)initWithFrame:(CGRect)frame videoScale:(CGFloat)scale;
- (void)displayFrame:(AVFrame *)frame;
- (AVFrame *)applyFilterToFrame:(AVFrame *)frame;
@end

NS_ASSUME_NONNULL_END
