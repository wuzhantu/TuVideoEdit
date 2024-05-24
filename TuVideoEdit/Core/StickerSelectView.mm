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
    self.iconImageNameArr = @[@"sticker_tiger", @"sticker_airplane"];
    self.titleArr = @[@"老虎", @"无人机"];
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.row) {
        case 0:
            VideoRenderConfig::shareInstance()->applyTigerSticker = true;
            VideoRenderConfig::shareInstance()->applyAirplaneSticker = false;
            break;
        case 1:
            VideoRenderConfig::shareInstance()->applyTigerSticker = false;
            VideoRenderConfig::shareInstance()->applyAirplaneSticker = true;
            break;
        default:
            break;
    }
}

@end
