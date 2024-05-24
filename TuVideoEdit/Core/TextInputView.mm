//
//  TextInputView.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/24.
//

#import "TextInputView.h"
#import "VideoRenderConfig.hpp"

@interface TextInputView ()
@property (nonatomic, strong) UITextField *textTF;
@end

@implementation TextInputView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setupView];
    }
    return self;
}

- (void)setupView {
    self.backgroundColor = [UIColor colorWithRed:27.0/255.0 green:37.0/255.0 blue:44.0/255.0 alpha:1];
    
    self.textTF = [[UITextField alloc] initWithFrame:CGRectMake(15, 0, self.bounds.size.width - 105, self.bounds.size.height)];
    self.textTF.attributedPlaceholder = [[NSAttributedString alloc] initWithString:@"请输入文字（暂时只支持英文）" attributes:@{NSForegroundColorAttributeName: UIColor.grayColor}];
    self.textTF.textColor = UIColor.whiteColor;
    [self addSubview:self.textTF];
    
    UIButton *confirmBtn = [[UIButton alloc] initWithFrame:CGRectMake(self.bounds.size.width - 75, (self.bounds.size.height - 40) * 0.5, 60, 40)];
    confirmBtn.layer.cornerRadius = 10;
    confirmBtn.clipsToBounds = YES;
    [confirmBtn setImage:[UIImage imageNamed:@"selected"] forState:UIControlStateNormal];
    confirmBtn.imageView.contentMode = UIViewContentModeScaleAspectFit;
    [confirmBtn addTarget:self action:@selector(confirmBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:confirmBtn];
}

- (void)confirmBtnAction {
    [self hide];
}

- (void)show {
    [self.textTF becomeFirstResponder];
    UIWindow *window = UIApplication.sharedApplication.keyWindow;
    [window addSubview:self];
    [UIView animateWithDuration:0.25 delay:0 options:UIViewAnimationOptionCurveEaseInOut animations:^{
        CGRect frame = self.frame;
        self.frame = CGRectMake(frame.origin.x, UIScreen.mainScreen.bounds.size.height - frame.size.height - 370, frame.size.width, frame.size.height);
    } completion:^(BOOL finished) {
        
    }];
}

- (void)hide {
    [self.textTF resignFirstResponder];
    if (self.textTF.text.length > 0) {
        const char *text = [self.textTF.text cStringUsingEncoding:NSUTF8StringEncoding];
        char *copyText = (char *)malloc(sizeof(char) * strlen(text));
        strcpy(copyText, text);
        VideoRenderConfig::shareInstance()->text = copyText;
    }
    
    [UIView animateWithDuration:0.25 delay:0 options:UIViewAnimationOptionCurveEaseInOut animations:^{
        CGRect frame = self.frame;
        self.frame = CGRectMake(frame.origin.x, UIScreen.mainScreen.bounds.size.height, frame.size.width, frame.size.height);
    } completion:^(BOOL finished) {
        self.textTF.text = @"";
        [self removeFromSuperview];
    }];
}

@end
