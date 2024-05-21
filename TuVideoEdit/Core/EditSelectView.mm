//
//  EditSelectView.m
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#import "EditSelectView.h"
#import "EditSelectCell.h"

@interface EditSelectView ()<UICollectionViewDelegate, UICollectionViewDataSource>
@property (nonatomic, strong) UICollectionView *collectionView;
@end

@implementation EditSelectView

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
    self.backgroundColor = [UIColor colorWithRed:27.0/255.0 green:37.0/255.0 blue:44.0/255.0 alpha:1];
    
    UIButton *confirmBtn = [[UIButton alloc] initWithFrame:CGRectMake(self.bounds.size.width - 75, 10, 60, 40)];
    confirmBtn.layer.cornerRadius = 10;
    confirmBtn.clipsToBounds = YES;
    [confirmBtn setImage:[UIImage imageNamed:@"selected"] forState:UIControlStateNormal];
    confirmBtn.imageView.contentMode = UIViewContentModeScaleAspectFit;
    [confirmBtn addTarget:self action:@selector(confirmBtnAction) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:confirmBtn];
    
    CGFloat screenWidth = UIScreen.mainScreen.bounds.size.width;
    int num = 4;
    CGFloat interitemSpacing = 40;
    UIEdgeInsets sectionInset = UIEdgeInsetsMake(0, 15, 0, 15);
    CGFloat itemWidth = floor((screenWidth - (num - 1) * interitemSpacing - sectionInset.left - sectionInset.right) / (CGFloat)num);
    CGFloat itemHeight = itemWidth + 30;
    UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
    layout.itemSize = CGSizeMake(itemWidth, itemHeight);
    layout.minimumLineSpacing = 0;
    layout.minimumInteritemSpacing = interitemSpacing;
    layout.sectionInset = sectionInset;
    layout.scrollDirection = UICollectionViewScrollDirectionVertical;
    self.collectionView = [[UICollectionView alloc] initWithFrame:CGRectMake(0, 60, self.bounds.size.width, self.bounds.size.height - 50) collectionViewLayout:layout];
    self.collectionView.delegate = self;
    self.collectionView.dataSource = self;
    self.collectionView.alwaysBounceVertical = YES;
    self.collectionView.prefetchingEnabled = NO;
    self.collectionView.backgroundColor = [UIColor colorWithRed:27.0/255.0 green:37.0/255.0 blue:44.0/255.0 alpha:1];
    [self.collectionView registerClass:EditSelectCell.class forCellWithReuseIdentifier:@"editSelectCell"];
    [self addSubview:self.collectionView];
}

#pragma mark - delegate
- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return self.titleArr.count;
}

- (__kindof UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    EditSelectCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"editSelectCell" forIndexPath:indexPath];
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView willDisplayCell:(UICollectionViewCell *)cell forItemAtIndexPath:(NSIndexPath *)indexPath {
    EditSelectCell *editCell = (EditSelectCell *)cell;
    editCell.iconImgView.image = [UIImage imageNamed:self.iconImageNameArr[indexPath.row]];
    editCell.titleLab.text = self.titleArr[indexPath.row];
}

- (void)show {
    UIWindow *window = UIApplication.sharedApplication.keyWindow;
    [window addSubview:self];
    [UIView animateWithDuration:0.5 animations:^{
        CGRect frame = self.frame;
        self.frame = CGRectMake(frame.origin.x, UIScreen.mainScreen.bounds.size.height - frame.size.height, frame.size.width, frame.size.height);
    }];
}

- (void)hide {
    [UIView animateWithDuration:0.5 animations:^{
        CGRect frame = self.frame;
        self.frame = CGRectMake(frame.origin.x, UIScreen.mainScreen.bounds.size.height, frame.size.width, frame.size.height);
    } completion:^(BOOL finished) {
        [self removeFromSuperview];
    }];
}

- (void)confirmBtnAction {
    [self hide];
}

@end
