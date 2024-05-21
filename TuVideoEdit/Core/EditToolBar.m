//
//  EditToolBar.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "EditToolBar.h"
#import "EditToolCell.h"
#import "EditSelectView.h"
#import "StickerSelectView.h"
#import "FilterSelectView.h"
#import "EffectSelectView.h"

@interface EditToolBar ()<UICollectionViewDelegate, UICollectionViewDataSource>
@property (nonatomic, strong) UICollectionView *collectionView;
@property (nonatomic, strong) NSArray *iconImageNameArr;
@property (nonatomic, strong) NSArray *titleArr;
@property (nonatomic, strong) StickerSelectView *stickerSelectView;
@property (nonatomic, strong) FilterSelectView *filterSelectView;
@property (nonatomic, strong) EffectSelectView *effectSelectView;
@end

@implementation EditToolBar

- (StickerSelectView *)stickerSelectView {
    if (!_stickerSelectView) {
        _stickerSelectView = [[StickerSelectView alloc] initWithFrame:CGRectMake(0, UIScreen.mainScreen.bounds.size.height, UIScreen.mainScreen.bounds.size.width, 350)];
    }
    return _stickerSelectView;
}

- (FilterSelectView *)filterSelectView {
    if (!_filterSelectView) {
        _filterSelectView = [[FilterSelectView alloc] initWithFrame:CGRectMake(0, UIScreen.mainScreen.bounds.size.height, UIScreen.mainScreen.bounds.size.width, 350)];
    }
    return _filterSelectView;
}

- (EffectSelectView *)effectSelectView {
    if (!_effectSelectView) {
        _effectSelectView = [[EffectSelectView alloc] initWithFrame:CGRectMake(0, UIScreen.mainScreen.bounds.size.height, UIScreen.mainScreen.bounds.size.width, 350)];
    }
    return _effectSelectView;
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setupData];
        [self setupView];
    }
    return self;
}

- (void)setupData {
    self.iconImageNameArr = @[@"toolbar_cut", @"toolbar_text", @"toolbar_sticker", @"toolbar_filter", @"toolbar_pictureInPicture", @"toolbar_effect"];
    self.titleArr = @[@"剪辑", @"文本", @"贴纸", @"滤镜", @"画中画", @"特效"];
}

- (void)setupView {
    CGFloat itemWidth = 40;
    CGFloat itemHeight = 40;
    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    layout.itemSize = CGSizeMake(itemWidth, itemHeight);
    layout.minimumLineSpacing = 30;
    layout.minimumInteritemSpacing = 0;
    layout.scrollDirection = UICollectionViewScrollDirectionHorizontal;
    self.collectionView = [[UICollectionView alloc] initWithFrame:self.bounds collectionViewLayout:layout];
    self.collectionView.delegate = self;
    self.collectionView.dataSource = self;
    self.collectionView.alwaysBounceHorizontal = YES;
    self.collectionView.showsHorizontalScrollIndicator = NO;
    self.collectionView.prefetchingEnabled = NO;
    self.collectionView.backgroundColor = [UIColor colorWithRed:27.0/255.0 green:37.0/255.0 blue:44.0/255.0 alpha:1];
    [self.collectionView registerClass:EditToolCell.class forCellWithReuseIdentifier:@"editToolCell"];
    [self addSubview:self.collectionView];
}

#pragma mark - delegate
- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return self.titleArr.count;
}

- (__kindof UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    EditToolCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"editToolCell" forIndexPath:indexPath];
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView willDisplayCell:(UICollectionViewCell *)cell forItemAtIndexPath:(NSIndexPath *)indexPath {
    EditToolCell *editCell = (EditToolCell *)cell;
    editCell.iconImgView.image = [UIImage imageNamed:self.iconImageNameArr[indexPath.row]];
    editCell.titleLab.text = self.titleArr[indexPath.row];
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.row) {
        case 2:
            [self.stickerSelectView show];
            break;
        case 3:
            [self.filterSelectView show];
            break;
        case 5:
            [self.effectSelectView show];
            break;
        default:
            break;
    }
}

@end
