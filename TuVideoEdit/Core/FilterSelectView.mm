//
//  FilterSelectView.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "FilterSelectView.h"
#include "VideoRenderConfig.hpp"

@implementation FilterSelectView

- (void)setupData {
    self.iconImageNameArr = @[@"toolbar_cut", @"toolbar_text"];
    self.titleArr = @[@"反相", @"灰度"];
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.row) {
        case 0:
            VideoRenderConfig::shareInstance()->applyInversionFilter = true;
            VideoRenderConfig::shareInstance()->applyGrayscaleFilter = false;
            break;
        case 1:
            VideoRenderConfig::shareInstance()->applyInversionFilter = false;
            VideoRenderConfig::shareInstance()->applyGrayscaleFilter = true;
            break;
        default:
            break;
    }
}

@end
