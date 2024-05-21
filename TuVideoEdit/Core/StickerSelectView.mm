//
//  StickerSelectView.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "StickerSelectView.h"
#include "VideoRenderConfig.hpp"

@implementation StickerSelectView

- (void)setupData {
    self.iconImageNameArr = @[@"toolbar_cut", @"toolbar_text"];
    self.titleArr = @[@"老虎", @"无人机"];
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.row) {
        case 0:
            VideoRenderConfig::shareInstance()->applySticker1 = true;
            VideoRenderConfig::shareInstance()->applySticker2 = false;
            break;
        case 1:
            VideoRenderConfig::shareInstance()->applySticker1 = false;
            VideoRenderConfig::shareInstance()->applySticker2 = true;
            break;
        default:
            break;
    }
}

@end
