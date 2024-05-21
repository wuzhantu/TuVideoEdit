//
//  EditToolCell.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "EditToolCell.h"

@implementation EditToolCell

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setupView];
    }
    return self;
}

- (void)setupView {
    self.iconImgView = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, self.contentView.frame.size.width, 20)];
    self.iconImgView.contentMode = UIViewContentModeScaleAspectFit;
    [self.contentView addSubview:self.iconImgView];
    
    self.titleLab = [[UILabel alloc] initWithFrame:CGRectMake(0, 28, self.contentView.frame.size.width, 10)];
    self.titleLab.textColor = UIColor.whiteColor;
    self.titleLab.font = [UIFont systemFontOfSize:10];
    self.titleLab.textAlignment = NSTextAlignmentCenter;
    [self.contentView addSubview:self.titleLab];
}

@end
