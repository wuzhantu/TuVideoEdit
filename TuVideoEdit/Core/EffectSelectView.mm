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
    self.iconImageNameArr = @[@"effect_mirror", @"effect_fourGrid"];
    self.titleArr = @[@"镜像", @"四宫格"];
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.row) {
        case 0:
            VideoRenderConfig::shareInstance()->applyMirrorEffect = true;
            VideoRenderConfig::shareInstance()->applyFourGridEffect = false;
            break;
        case 1:
            VideoRenderConfig::shareInstance()->applyMirrorEffect = false;
            VideoRenderConfig::shareInstance()->applyFourGridEffect = true;
            break;
        default:
            break;
    }
}

@end
