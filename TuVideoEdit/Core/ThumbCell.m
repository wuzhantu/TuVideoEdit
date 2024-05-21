//
//  ThumbCell.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/4/29.
//

#import "ThumbCell.h"

@interface ThumbCell ()

@end

@implementation ThumbCell

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        NSLog(@"setupCell");
        [self setupView];
    }
    return self;
}

- (void)setupView {
    self.thumbImgView = [[UIImageView alloc] initWithFrame:self.contentView.bounds];
    self.thumbImgView.contentMode = UIViewContentModeScaleAspectFill;
    self.thumbImgView.clipsToBounds = YES;
    self.thumbImgView.backgroundColor = [UIColor colorWithRed:27.0/255.0 green:37.0/255.0 blue:44.0/255.0 alpha:1];
    [self.contentView addSubview:self.thumbImgView];
}

@end
