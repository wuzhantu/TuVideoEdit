//
//  EditSelectCell.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "EditSelectCell.h"

@implementation EditSelectCell

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setupView];
    }
    return self;
}

- (void)setupView {
    self.iconImgView = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, self.contentView.frame.size.width, self.contentView.frame.size.width)];
    self.iconImgView.contentMode = UIViewContentModeScaleAspectFill;
    self.iconImgView.layer.cornerRadius = 10;
    self.iconImgView.layer.masksToBounds = YES;
    [self.contentView addSubview:self.iconImgView];
    
    self.titleLab = [[UILabel alloc] initWithFrame:CGRectMake(0, CGRectGetMaxY(self.iconImgView.frame) + 10, self.contentView.frame.size.width, 20)];
    self.titleLab.textColor = UIColor.whiteColor;
    self.titleLab.font = [UIFont systemFontOfSize:10];
    self.titleLab.textAlignment = NSTextAlignmentCenter;
    [self.contentView addSubview:self.titleLab];
}

@end
