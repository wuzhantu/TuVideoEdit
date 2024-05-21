//
//  EditSelectView.h
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface EditSelectView : UIView
@property (nonatomic, strong) NSArray *iconImageNameArr;
@property (nonatomic, strong) NSArray *titleArr;
- (void)setupData;
- (void)show;
- (void)hide;
@end

NS_ASSUME_NONNULL_END
