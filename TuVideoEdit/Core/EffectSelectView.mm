//
//  EffectSelectView.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "EffectSelectView.h"
#include "VideoRenderConfig.hpp"

@implementation EffectSelectView

- (void)setupData {
    self.iconImageNameArr = @[@"toolbar_cut", @"toolbar_text"];
    self.titleArr = @[@"二分", @"四分"];
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.row) {
        case 0:
            VideoRenderConfig::shareInstance()->applyEffect1 = true;
            VideoRenderConfig::shareInstance()->applyEffect2 = false;
            break;
        case 1:
            VideoRenderConfig::shareInstance()->applyEffect1 = false;
            VideoRenderConfig::shareInstance()->applyEffect2 = true;
            break;
        default:
            break;
    }
}

@end
